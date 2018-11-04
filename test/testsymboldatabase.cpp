/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2018 Cppcheck team.
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

#include "platform.h"
#include "settings.h"
#include "symboldatabase.h"
#include "testsuite.h"
#include "testutils.h"
#include "token.h"
#include "tokenize.h"
#include "tokenlist.h"
#include "utils.h"

#include <climits>
#include <cstddef>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

struct InternalError;

#define GET_SYMBOL_DB(code) \
    Tokenizer tokenizer(&settings1, this); \
    const SymbolDatabase *db = getSymbolDB_inner(tokenizer, code, "test.cpp");

#define GET_SYMBOL_DB_C(code) \
    Tokenizer tokenizer(&settings1, this); \
    const SymbolDatabase *db = getSymbolDB_inner(tokenizer, code, "test.c");

class TestSymbolDatabase: public TestFixture {
public:
    TestSymbolDatabase()
        :TestFixture("TestSymbolDatabase")
        ,nullScope(nullptr, nullptr, nullptr)
        ,vartok(nullptr)
        ,typetok(nullptr) {
    }

private:
    const Scope nullScope;
    const Token* vartok;
    const Token* typetok;
    Settings settings1;
    Settings settings2;

    void reset() {
        vartok = nullptr;
        typetok = nullptr;
    }

    const static SymbolDatabase* getSymbolDB_inner(Tokenizer& tokenizer, const char* code, const char* filename) {
        errout.str("");
        std::istringstream istr(code);
        tokenizer.tokenize(istr, filename);
        return tokenizer.getSymbolDatabase();
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

    void run() override {
        LOAD_LIB_2(settings1.library, "std.cfg");
        settings2.platform(Settings::Unspecified);

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
        TEST_CASE(test_isVariableDeclarationIdentifiesPointerArray);
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
        TEST_CASE(isVariableDeclarationDoesNotIdentifyCase);
        TEST_CASE(isVariableStlType);
        TEST_CASE(isVariablePointerToConstPointer);
        TEST_CASE(isVariablePointerToVolatilePointer);
        TEST_CASE(isVariablePointerToConstVolatilePointer);
        TEST_CASE(isVariableMultiplePointersAndQualifiers);

        TEST_CASE(VariableValueType1);
        TEST_CASE(VariableValueType2);
        TEST_CASE(VariableValueType3);

        TEST_CASE(findVariableType1);
        TEST_CASE(findVariableType2);
        TEST_CASE(findVariableType3);

        TEST_CASE(rangeBasedFor);

        TEST_CASE(arrayMemberVar1);
        TEST_CASE(arrayMemberVar2);
        TEST_CASE(arrayMemberVar3);
        TEST_CASE(staticMemberVar);
        TEST_CASE(getVariableFromVarIdBoundsCheck);

        TEST_CASE(hasRegularFunction);
        TEST_CASE(hasRegularFunction_trailingReturnType);
        TEST_CASE(hasInlineClassFunction);
        TEST_CASE(hasInlineClassFunction_trailingReturnType);
        TEST_CASE(hasMissingInlineClassFunction);
        TEST_CASE(hasClassFunction);
        TEST_CASE(hasClassFunction_trailingReturnType);

        TEST_CASE(hasRegularFunctionReturningFunctionPointer);
        TEST_CASE(hasInlineClassFunctionReturningFunctionPointer);
        TEST_CASE(hasMissingInlineClassFunctionReturningFunctionPointer);
        TEST_CASE(hasInlineClassOperatorTemplate);
        TEST_CASE(hasClassFunctionReturningFunctionPointer);
        TEST_CASE(methodWithRedundantScope);
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
        TEST_CASE(checkTypeStartEndToken3); // no variable name: void f(const char){}

        TEST_CASE(functionArgs1);
        TEST_CASE(functionArgs2);
        TEST_CASE(functionArgs3);
        TEST_CASE(functionArgs4);
        TEST_CASE(functionArgs5); // #7650
        TEST_CASE(functionArgs6); // #7651
        TEST_CASE(functionArgs7); // #7652
        TEST_CASE(functionArgs8); // #7653
        TEST_CASE(functionArgs9); // #7657
        TEST_CASE(functionArgs10);
        TEST_CASE(functionArgs11);
        TEST_CASE(functionArgs12); // #7661
        TEST_CASE(functionArgs13); // #7697

        TEST_CASE(functionImplicitlyVirtual);

        TEST_CASE(namespaces1);
        TEST_CASE(namespaces2);
        TEST_CASE(namespaces3);  // #3854 - unknown macro
        TEST_CASE(namespaces4);

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
        TEST_CASE(symboldatabase17); // ticket #2657
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
        TEST_CASE(symboldatabase52); // #6581
        TEST_CASE(symboldatabase53); // #7124 (library podtype)
        TEST_CASE(symboldatabase54); // #7257
        TEST_CASE(symboldatabase55); // #7767 (return unknown macro)
        TEST_CASE(symboldatabase56); // #7909
        TEST_CASE(symboldatabase57);
        TEST_CASE(symboldatabase58); // #6985 (using namespace type lookup)
        TEST_CASE(symboldatabase59);
        TEST_CASE(symboldatabase60);
        TEST_CASE(symboldatabase61);
        TEST_CASE(symboldatabase62);
        TEST_CASE(symboldatabase63);
        TEST_CASE(symboldatabase64);
        TEST_CASE(symboldatabase65);
        TEST_CASE(symboldatabase66); // #8540
        TEST_CASE(symboldatabase67); // #8538
        TEST_CASE(symboldatabase68); // #8560
        TEST_CASE(symboldatabase69);
        TEST_CASE(symboldatabase70);
        TEST_CASE(symboldatabase71);
        TEST_CASE(symboldatabase72); // #8600
        TEST_CASE(symboldatabase73); // #8603

        TEST_CASE(createSymbolDatabaseFindAllScopes1);

        TEST_CASE(enum1);
        TEST_CASE(enum2);
        TEST_CASE(enum3);
        TEST_CASE(enum4);
        TEST_CASE(enum5);
        TEST_CASE(enum6);
        TEST_CASE(enum7);

        TEST_CASE(sizeOfType);

        TEST_CASE(isImplicitlyVirtual);
        TEST_CASE(isPure);

        TEST_CASE(isFunction1); // UNKNOWN_MACRO(a,b) { .. }
        TEST_CASE(isFunction2);

        TEST_CASE(findFunction1);
        TEST_CASE(findFunction2); // mismatch: parameter passed by address => reference argument
        TEST_CASE(findFunction3);
        TEST_CASE(findFunction4);
        TEST_CASE(findFunction5); // #6230
        TEST_CASE(findFunction6);
        TEST_CASE(findFunction7); // #6700
        TEST_CASE(findFunction8);
        TEST_CASE(findFunction9);
        TEST_CASE(findFunction10); // #7673
        TEST_CASE(findFunction11);
        TEST_CASE(findFunction12);
        TEST_CASE(findFunction13);
        TEST_CASE(findFunction14);
        TEST_CASE(findFunction15);
        TEST_CASE(findFunction16);
        TEST_CASE(findFunction17);
        TEST_CASE(findFunction18);
        TEST_CASE(findFunction19);
        TEST_CASE(findFunction20); // #8280

        TEST_CASE(noexceptFunction1);
        TEST_CASE(noexceptFunction2);
        TEST_CASE(noexceptFunction3);
        TEST_CASE(noexceptFunction4);

        TEST_CASE(throwFunction1);
        TEST_CASE(throwFunction2);

        TEST_CASE(nothrowAttributeFunction);
        TEST_CASE(nothrowDeclspecFunction);

        TEST_CASE(noreturnAttributeFunction);

        TEST_CASE(varTypesIntegral); // known integral
        TEST_CASE(varTypesFloating); // known floating
        TEST_CASE(varTypesOther);    // (un)known

        TEST_CASE(functionPrototype); // #5867

        TEST_CASE(lambda); // #5867
        TEST_CASE(lambda2); // #7473

        TEST_CASE(circularDependencies); // #6298

        TEST_CASE(executableScopeWithUnknownFunction);

        TEST_CASE(valuetype);

        TEST_CASE(variadic1); // #7453
        TEST_CASE(variadic2); // #7649
        TEST_CASE(variadic3); // #7387

        TEST_CASE(noReturnType);

        TEST_CASE(auto1);
        TEST_CASE(auto2);
        TEST_CASE(auto3);
        TEST_CASE(auto4);
        TEST_CASE(auto5);
        TEST_CASE(auto6); // #7963 (segmentation fault)
        TEST_CASE(auto7);
        TEST_CASE(auto8);
        TEST_CASE(auto9); // #8044 (segmentation fault)
        TEST_CASE(auto10); // #8020

        TEST_CASE(unionWithConstructor);

        TEST_CASE(using1);
        TEST_CASE(using2); // #8331 (segmentation fault)
        TEST_CASE(using3); // #8343 (segmentation fault)
    }

    void array() {
        GET_SYMBOL_DB_C("int a[10+2];");
        ASSERT(db != nullptr);
        if (!db)
            return;
        ASSERT(db->variableList().size() == 2); // the first one is not used
        const Variable * v = db->getVariableFromVarId(1);
        ASSERT(v != nullptr);
        if (!v)
            return;
        ASSERT(v->isArray());
        ASSERT_EQUALS(1U, v->dimensions().size());
        ASSERT_EQUALS(12U, v->dimension(0));
    }

    void stlarray() const {
        std::istringstream code("std::array<int, 20> arr;");
        TokenList list(nullptr);
        list.createTokens(code, "test.c");
        list.front()->tokAt(3)->link(list.front()->tokAt(7));
        Variable v(list.front()->next(), list.front(), list.back(), 0, Public, nullptr, nullptr, &settings1);
        ASSERT(v.isArray());
        ASSERT_EQUALS(1U, v.dimensions().size());
        ASSERT_EQUALS(20U, v.dimension(0));
    }

    void test_isVariableDeclarationCanHandleNull() {
        reset();
        const bool result = nullScope.isVariableDeclaration(nullptr, vartok, typetok);
        ASSERT_EQUALS(false, result);
        ASSERT(nullptr == vartok);
        ASSERT(nullptr == typetok);
        Variable v(nullptr, nullptr, nullptr, 0, Public, 0, 0, &settings1);
    }

    void test_isVariableDeclarationIdentifiesSimpleDeclaration() {
        reset();
        givenACodeSampleToTokenize simpleDeclaration("int x;");
        const bool result = nullScope.isVariableDeclaration(simpleDeclaration.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("x", vartok->str());
        ASSERT_EQUALS("int", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0, &settings1);
        ASSERT(false == v.isArray());
        ASSERT(false == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void test_isVariableDeclarationIdentifiesInitialization() {
        reset();
        givenACodeSampleToTokenize simpleDeclaration("int x (1);");
        const bool result = nullScope.isVariableDeclaration(simpleDeclaration.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("x", vartok->str());
        ASSERT_EQUALS("int", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0, &settings1);
        ASSERT(false == v.isArray());
        ASSERT(false == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void test_isVariableDeclarationIdentifiesCpp11Initialization() {
        reset();
        givenACodeSampleToTokenize simpleDeclaration("int x {1};");
        const bool result = nullScope.isVariableDeclaration(simpleDeclaration.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("x", vartok->str());
        ASSERT_EQUALS("int", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0, &settings1);
        ASSERT(false == v.isArray());
        ASSERT(false == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void test_isVariableDeclarationIdentifiesScopedDeclaration() {
        reset();
        givenACodeSampleToTokenize ScopedDeclaration("::int x;");
        const bool result = nullScope.isVariableDeclaration(ScopedDeclaration.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("x", vartok->str());
        ASSERT_EQUALS("int", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0, &settings1);
        ASSERT(false == v.isArray());
        ASSERT(false == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void test_isVariableDeclarationIdentifiesStdDeclaration() {
        reset();
        givenACodeSampleToTokenize StdDeclaration("std::string x;");
        const bool result = nullScope.isVariableDeclaration(StdDeclaration.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("x", vartok->str());
        ASSERT_EQUALS("string", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0, &settings1);
        ASSERT(false == v.isArray());
        ASSERT(false == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void test_isVariableDeclarationIdentifiesScopedStdDeclaration() {
        reset();
        givenACodeSampleToTokenize StdDeclaration("::std::string x;");
        const bool result = nullScope.isVariableDeclaration(StdDeclaration.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("x", vartok->str());
        ASSERT_EQUALS("string", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0, &settings1);
        ASSERT(false == v.isArray());
        ASSERT(false == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void test_isVariableDeclarationIdentifiesManyScopes() {
        reset();
        givenACodeSampleToTokenize manyScopes("AA::BB::CC::DD::EE x;");
        const bool result = nullScope.isVariableDeclaration(manyScopes.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("x", vartok->str());
        ASSERT_EQUALS("EE", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0, &settings1);
        ASSERT(false == v.isArray());
        ASSERT(false == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void test_isVariableDeclarationIdentifiesPointers() {
        reset();
        givenACodeSampleToTokenize pointer("int* p;");
        const bool result1 = nullScope.isVariableDeclaration(pointer.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result1);
        ASSERT_EQUALS("p", vartok->str());
        ASSERT_EQUALS("int", typetok->str());
        Variable v1(vartok, typetok, vartok->previous(), 0, Public, 0, 0, &settings1);
        ASSERT(false == v1.isArray());
        ASSERT(true == v1.isPointer());
        ASSERT(false == v1.isReference());

        reset();
        givenACodeSampleToTokenize constpointer("const int* p;");
        Variable v2(constpointer.tokens()->tokAt(3), constpointer.tokens()->next(), constpointer.tokens()->tokAt(2), 0, Public, 0, 0, &settings1);
        ASSERT(false == v2.isArray());
        ASSERT(true == v2.isPointer());
        ASSERT(false == v2.isConst());
        ASSERT(false == v2.isReference());

        reset();
        givenACodeSampleToTokenize pointerconst("int* const p;");
        const bool result2 = nullScope.isVariableDeclaration(pointerconst.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result2);
        ASSERT_EQUALS("p", vartok->str());
        ASSERT_EQUALS("int", typetok->str());
        Variable v3(vartok, typetok, vartok->previous(), 0, Public, 0, 0, &settings1);
        ASSERT(false == v3.isArray());
        ASSERT(true == v3.isPointer());
        ASSERT(true == v3.isConst());
        ASSERT(false == v3.isReference());
    }

    void test_isVariableDeclarationDoesNotIdentifyConstness() {
        reset();
        givenACodeSampleToTokenize constness("const int* cp;");
        const bool result = nullScope.isVariableDeclaration(constness.tokens(), vartok, typetok);
        ASSERT_EQUALS(false, result);
        ASSERT(nullptr == vartok);
        ASSERT(nullptr == typetok);
    }

    void test_isVariableDeclarationIdentifiesFirstOfManyVariables() {
        reset();
        givenACodeSampleToTokenize multipleDeclaration("int first, second;");
        const bool result = nullScope.isVariableDeclaration(multipleDeclaration.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("first", vartok->str());
        ASSERT_EQUALS("int", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0, &settings1);
        ASSERT(false == v.isArray());
        ASSERT(false == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void test_isVariableDeclarationIdentifiesScopedPointerDeclaration() {
        reset();
        givenACodeSampleToTokenize manyScopes("AA::BB::CC::DD::EE* p;");
        const bool result = nullScope.isVariableDeclaration(manyScopes.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("p", vartok->str());
        ASSERT_EQUALS("EE", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0, &settings1);
        ASSERT(false == v.isArray());
        ASSERT(true == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void test_isVariableDeclarationIdentifiesDeclarationWithIndirection() {
        reset();
        givenACodeSampleToTokenize pointerToPointer("int** pp;");
        const bool result = nullScope.isVariableDeclaration(pointerToPointer.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("pp", vartok->str());
        ASSERT_EQUALS("int", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0, &settings1);
        ASSERT(false == v.isArray());
        ASSERT(true == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void test_isVariableDeclarationIdentifiesDeclarationWithMultipleIndirection() {
        reset();
        givenACodeSampleToTokenize pointerToPointer("int***** p;");
        const bool result = nullScope.isVariableDeclaration(pointerToPointer.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("p", vartok->str());
        ASSERT_EQUALS("int", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0, &settings1);
        ASSERT(false == v.isArray());
        ASSERT(true == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void test_isVariableDeclarationIdentifiesArray() {
        reset();
        givenACodeSampleToTokenize arr("::std::string v[3];");
        const bool result = nullScope.isVariableDeclaration(arr.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("v", vartok->str());
        ASSERT_EQUALS("string", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0, &settings1);
        ASSERT(true == v.isArray());
        ASSERT(false == v.isPointer());
        ASSERT(false == v.isPointerArray());
        ASSERT(false == v.isReference());
    }

    void test_isVariableDeclarationIdentifiesPointerArray() {
        reset();
        givenACodeSampleToTokenize arr("A *a[5];");
        const bool result = nullScope.isVariableDeclaration(arr.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("a", vartok->str());
        ASSERT_EQUALS("A", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0, &settings1);
        ASSERT(false == v.isPointer());
        ASSERT(true == v.isArray());
        ASSERT(false == v.isPointerToArray());
        ASSERT(true == v.isPointerArray());
        ASSERT(false == v.isReference());
    }

    void test_isVariableDeclarationIdentifiesOfArrayPointers() {
        reset();
        givenACodeSampleToTokenize arr("A (*a)[5];");
        const bool result = nullScope.isVariableDeclaration(arr.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("a", vartok->str());
        ASSERT_EQUALS("A", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0, &settings1);
        ASSERT(true == v.isPointer());
        ASSERT(false == v.isArray());
        ASSERT(true == v.isPointerToArray());
        ASSERT(false == v.isPointerArray());
        ASSERT(false == v.isReference());
    }

    void isVariableDeclarationIdentifiesTemplatedPointerVariable() {
        reset();
        givenACodeSampleToTokenize var("std::set<char>* chars;");
        const bool result = nullScope.isVariableDeclaration(var.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("chars", vartok->str());
        ASSERT_EQUALS("set", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0, &settings1);
        ASSERT(false == v.isArray());
        ASSERT(true == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void isVariableDeclarationIdentifiesTemplatedPointerToPointerVariable() {
        reset();
        givenACodeSampleToTokenize var("std::deque<int>*** ints;");
        const bool result = nullScope.isVariableDeclaration(var.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("ints", vartok->str());
        ASSERT_EQUALS("deque", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0, &settings1);
        ASSERT(false == v.isArray());
        ASSERT(true == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void isVariableDeclarationIdentifiesTemplatedArrayVariable() {
        reset();
        givenACodeSampleToTokenize var("std::deque<int> ints[3];");
        const bool result = nullScope.isVariableDeclaration(var.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("ints", vartok->str());
        ASSERT_EQUALS("deque", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0, &settings1);
        ASSERT(true == v.isArray());
        ASSERT(false == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void isVariableDeclarationIdentifiesTemplatedVariable() {
        reset();
        givenACodeSampleToTokenize var("std::vector<int> ints;");
        const bool result = nullScope.isVariableDeclaration(var.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("ints", vartok->str());
        ASSERT_EQUALS("vector", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0, &settings1);
        ASSERT(false == v.isArray());
        ASSERT(false == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void isVariableDeclarationIdentifiesTemplatedVariableIterator() {
        reset();
        givenACodeSampleToTokenize var("std::list<int>::const_iterator floats;");
        const bool result = nullScope.isVariableDeclaration(var.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("floats", vartok->str());
        ASSERT_EQUALS("const_iterator", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0, &settings1);
        ASSERT(false == v.isArray());
        ASSERT(false == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void isVariableDeclarationIdentifiesNestedTemplateVariable() {
        reset();
        givenACodeSampleToTokenize var("std::deque<std::set<int> > intsets;");
        const bool result = nullScope.isVariableDeclaration(var.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("intsets", vartok->str());
        ASSERT_EQUALS("deque", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0, &settings1);
        ASSERT(false == v.isArray());
        ASSERT(false == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void isVariableDeclarationIdentifiesReference() {
        reset();
        givenACodeSampleToTokenize var1("int& foo;");
        const bool result1 = nullScope.isVariableDeclaration(var1.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result1);
        Variable v1(vartok, typetok, vartok->previous(), 0, Public, 0, 0, &settings1);
        ASSERT(false == v1.isArray());
        ASSERT(false == v1.isPointer());
        ASSERT(true == v1.isReference());

        reset();
        givenACodeSampleToTokenize var2("foo*& bar;");
        const bool result2 = nullScope.isVariableDeclaration(var2.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result2);
        Variable v2(vartok, typetok, vartok->previous(), 0, Public, 0, 0, &settings1);
        ASSERT(false == v2.isArray());
        ASSERT(true == v2.isPointer());
        ASSERT(true == v2.isReference());

        reset();
        givenACodeSampleToTokenize var3("std::vector<int>& foo;");
        const bool result3 = nullScope.isVariableDeclaration(var3.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result3);
        Variable v3(vartok, typetok, vartok->previous(), 0, Public, 0, 0, &settings1);
        ASSERT(false == v3.isArray());
        ASSERT(false == v3.isPointer());
        ASSERT(true == v3.isReference());
    }

    void isVariableDeclarationDoesNotIdentifyTemplateClass() {
        reset();
        givenACodeSampleToTokenize var("template <class T> class SomeClass{};");
        const bool result = nullScope.isVariableDeclaration(var.tokens(), vartok, typetok);
        ASSERT_EQUALS(false, result);
    }

    void isVariableDeclarationDoesNotIdentifyCppCast() {
        reset();
        givenACodeSampleToTokenize var("reinterpret_cast <char *> (code)[0] = 0;");
        const bool result = nullScope.isVariableDeclaration(var.tokens(), vartok, typetok);
        ASSERT_EQUALS(false, result);
    }

    void isVariableDeclarationPointerConst() {
        reset();
        givenACodeSampleToTokenize var("std::string const* s;");
        const bool result = nullScope.isVariableDeclaration(var.tokens()->next(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0, &settings1);
        ASSERT(false == v.isArray());
        ASSERT(true == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void isVariableDeclarationRValueRef() {
        reset();
        givenACodeSampleToTokenize var("int&& i;");
        const bool result = nullScope.isVariableDeclaration(var.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0, &settings1);
        ASSERT(false == v.isArray());
        ASSERT(false == v.isPointer());
        ASSERT(true == v.isReference());
        ASSERT(true == v.isRValueReference());
        ASSERT(var.tokens()->tokAt(2)->scope() != 0);
    }

    void isVariableDeclarationDoesNotIdentifyCase() {
        GET_SYMBOL_DB_C("a b;\n"
                        "void f() {\n"
                        "  switch (c) {\n"
                        "    case b:;\n"
                        "  }"
                        "}");
        const Variable* b = db->getVariableFromVarId(1);
        ASSERT_EQUALS("b", b->name());
        ASSERT_EQUALS("a", b->typeStartToken()->str());
    }

    void VariableValueType1() {
        GET_SYMBOL_DB("typedef uint8_t u8;\n"
                      "static u8 x;\n");
        const Variable* x = db->getVariableFromVarId(1);
        ASSERT_EQUALS("x", x->name());
        ASSERT(x->valueType()->isIntegral());
    }

    void VariableValueType2() {
        GET_SYMBOL_DB("using u8 = uint8_t;\n"
                      "static u8 x;\n");
        const Variable* x = db->getVariableFromVarId(1);
        ASSERT_EQUALS("x", x->name());
        ASSERT(x->valueType()->isIntegral());
    }

    void VariableValueType3() {
        GET_SYMBOL_DB("void f(std::string::size_type x);\n");
        const Variable* x = db->getVariableFromVarId(1);
        ASSERT_EQUALS("x", x->name());
        // TODO: Configure std::string::size_type somehow.
        TODO_ASSERT_EQUALS(ValueType::Type::LONGLONG, ValueType::Type::UNKNOWN_TYPE, x->valueType()->type);
    }

    void findVariableType1() {
        GET_SYMBOL_DB("class A {\n"
                      "public:\n"
                      "    struct B {};\n"
                      "    void f();\n"
                      "};\n"
                      "\n"
                      "void f()\n"
                      "{\n"
                      "    struct A::B b;\n"
                      "    b.x = 1;\n"
                      "}");
        ASSERT(db != nullptr);
        if (!db)
            return;
        const Variable* bvar = db->getVariableFromVarId(1);
        ASSERT_EQUALS("b", bvar->name());
        ASSERT(bvar->type() != nullptr);
    }

    void findVariableType2() {
        GET_SYMBOL_DB("class A {\n"
                      "public:\n"
                      "    class B {\n"
                      "    public:\n"
                      "        struct C {\n"
                      "            int x;\n"
                      "            int y;\n"
                      "        };\n"
                      "    };\n"
                      "\n"
                      "    void f();\n"
                      "};\n"
                      "\n"
                      "void A::f()\n"
                      "{\n"
                      "    struct B::C c;\n"
                      "    c.x = 1;\n"
                      "}");
        ASSERT(db != nullptr);
        if (!db)
            return;
        const Variable* cvar = db->getVariableFromVarId(3);
        ASSERT_EQUALS("c", cvar->name());
        ASSERT(cvar->type() != nullptr);
    }

    void findVariableType3() {
        GET_SYMBOL_DB("namespace {\n"
                      "    struct A {\n"
                      "        int x;\n"
                      "        int y;\n"
                      "    };\n"
                      "}\n"
                      "\n"
                      "void f()\n"
                      "{\n"
                      "    struct A a;\n"
                      "    a.x = 1;\n"
                      "}");
        (void)db;
        const Variable* avar = Token::findsimplematch(tokenizer.tokens(), "a")->variable();
        ASSERT(avar);
        ASSERT(avar && avar->type() != nullptr);
    }

    void rangeBasedFor() {
        GET_SYMBOL_DB("void reset() {\n"
                      "    for(auto& e : array)\n"
                      "        foo(e);\n"
                      "}");

        ASSERT(db != nullptr);
        if (!db)
            return;
        ASSERT(db->scopeList.back().type == Scope::eFor);
        ASSERT_EQUALS(2, db->variableList().size());
        if (db->variableList().size() < 2)
            return;
        const Variable* e = db->getVariableFromVarId(1);
        ASSERT(e && e->isReference() && e->isLocal());
    }
    void isVariableStlType() {
        {
            reset();
            std::istringstream code("std::string s;");
            TokenList list(nullptr);
            list.createTokens(code, "test.cpp");
            const bool result = nullScope.isVariableDeclaration(list.front(), vartok, typetok);
            ASSERT_EQUALS(true, result);
            Variable v(vartok, list.front(), list.back(), 0, Public, 0, 0, &settings1);
            static const std::set<std::string> types = { "string", "wstring" };
            static const std::set<std::string> no_types = { "set" };
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
            const bool result = nullScope.isVariableDeclaration(list.front(), vartok, typetok);
            ASSERT_EQUALS(true, result);
            Variable v(vartok, list.front(), list.back(), 0, Public, 0, 0, &settings1);
            static const std::set<std::string> types = { "bitset", "set", "vector", "wstring" };
            static const std::set<std::string> no_types = { "bitset", "map", "set" };
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
            const bool result = nullScope.isVariableDeclaration(list.front(), vartok, typetok);
            ASSERT_EQUALS(true, result);
            Variable v(vartok, list.front(), list.back(), 0, Public, 0, 0, &settings1);
            static const std::set<std::string> types = { "bitset", "set", "vector" };
            ASSERT_EQUALS(false, v.isStlType());
            ASSERT_EQUALS(false, v.isStlType(types));
            ASSERT_EQUALS(false, v.isStlStringType());
        }
    }

    void isVariablePointerToConstPointer() {
        reset();
        givenACodeSampleToTokenize var("char* const * s;");
        bool result = nullScope.isVariableDeclaration(var.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0, &settings1);
        ASSERT(false == v.isArray());
        ASSERT(true == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void isVariablePointerToVolatilePointer() {
        reset();
        givenACodeSampleToTokenize var("char* volatile * s;");
        bool result = nullScope.isVariableDeclaration(var.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0, &settings1);
        ASSERT(false == v.isArray());
        ASSERT(true == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void isVariablePointerToConstVolatilePointer() {
        reset();
        givenACodeSampleToTokenize var("char* const volatile * s;");
        bool result = nullScope.isVariableDeclaration(var.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0, &settings1);
        ASSERT(false == v.isArray());
        ASSERT(true == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void isVariableMultiplePointersAndQualifiers() {
        reset();
        givenACodeSampleToTokenize var("const char* const volatile * const volatile * const volatile * const volatile s;");
        bool result = nullScope.isVariableDeclaration(var.tokens()->next(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        Variable v(vartok, typetok, vartok->previous(), 0, Public, 0, 0, &settings1);
        ASSERT(false == v.isArray());
        ASSERT(true == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void arrayMemberVar1() {
        GET_SYMBOL_DB("struct Foo {\n"
                      "    int x;\n"
                      "};\n"
                      "void f() {\n"
                      "    struct Foo foo[10];\n"
                      "    foo[1].x = 123;\n"  // <- x should get a variable() pointer
                      "}");

        const Token *tok = Token::findsimplematch(tokenizer.tokens(), ". x");
        tok = tok ? tok->next() : nullptr;
        ASSERT(db != nullptr);
        ASSERT(tok && tok->variable() && Token::simpleMatch(tok->variable()->typeStartToken(), "int x ;"));
        ASSERT(tok && tok->varId() == 3U); // It's possible to set a varId
    }

    void arrayMemberVar2() {
        GET_SYMBOL_DB("struct Foo {\n"
                      "    int x;\n"
                      "};\n"
                      "void f() {\n"
                      "    struct Foo foo[10][10];\n"
                      "    foo[1][2].x = 123;\n"  // <- x should get a variable() pointer
                      "}");

        const Token *tok = Token::findsimplematch(tokenizer.tokens(), ". x");
        tok = tok ? tok->next() : nullptr;
        ASSERT(db != nullptr);
        ASSERT(tok && tok->variable() && Token::simpleMatch(tok->variable()->typeStartToken(), "int x ;"));
        ASSERT(tok && tok->varId() == 3U); // It's possible to set a varId
    }

    void arrayMemberVar3() {
        GET_SYMBOL_DB("struct Foo {\n"
                      "    int x;\n"
                      "};\n"
                      "void f() {\n"
                      "    struct Foo foo[10];\n"
                      "    (foo[1]).x = 123;\n"  // <- x should get a variable() pointer
                      "}");

        const Token *tok = Token::findsimplematch(tokenizer.tokens(), ". x");
        tok = tok ? tok->next() : nullptr;
        ASSERT(db != nullptr);
        ASSERT(tok && tok->variable() && Token::simpleMatch(tok->variable()->typeStartToken(), "int x ;"));
        ASSERT(tok && tok->varId() == 3U); // It's possible to set a varId
    }

    void staticMemberVar() {
        GET_SYMBOL_DB("class Foo {\n"
                      "    static const double d;\n"
                      "};\n"
                      "const double Foo::d = 5.0;");

        const Variable* v = db->getVariableFromVarId(1);
        ASSERT(v && db->variableList().size() == 2);
        ASSERT(v && v->isStatic() && v->isConst() && v->isPrivate());
    }

    void getVariableFromVarIdBoundsCheck() {
        GET_SYMBOL_DB("int x;\n"
                      "int y;\n");

        const Variable* v = db->getVariableFromVarId(2);
        // three elements: varId 0 also counts via a fake-entry
        ASSERT(v && db->variableList().size() == 3);

        ASSERT_THROW(db->getVariableFromVarId(3), std::out_of_range);
    }

    void hasRegularFunction() {
        GET_SYMBOL_DB("void func() { }\n")

        // 2 scopes: Global and Function
        ASSERT(db && db->scopeList.size() == 2);

        if (db) {
            const Scope *scope = findFunctionScopeByToken(db, tokenizer.tokens()->next());

            ASSERT(scope && scope->className == "func");
            if (!scope)
                return;
            ASSERT(scope->functionOf == 0);

            const Function *function = findFunctionByName("func", &db->scopeList.front());

            ASSERT(function && function->token->str() == "func");
            ASSERT(function && function->token == tokenizer.tokens()->next());
            ASSERT(function && function->hasBody());
            ASSERT(function && function->functionScope == scope && scope->function == function && function->nestedIn != scope);
            ASSERT(function && function->retDef == tokenizer.tokens());
        }
    }

    void hasRegularFunction_trailingReturnType() {
        GET_SYMBOL_DB("auto func() -> int { }")

        // 2 scopes: Global and Function
        ASSERT(db && db->scopeList.size() == 2);

        if (db) {
            const Scope *scope = findFunctionScopeByToken(db, tokenizer.tokens()->next());

            ASSERT(scope && scope->className == "func");
            if (!scope)
                return;
            ASSERT(scope->functionOf == 0);

            const Function *function = findFunctionByName("func", &db->scopeList.front());

            ASSERT(function && function->token->str() == "func");
            ASSERT(function && function->token == tokenizer.tokens()->next());
            ASSERT(function && function->hasBody());
            ASSERT(function && function->functionScope == scope && scope->function == function && function->nestedIn != scope);
            ASSERT(function && function->retDef == tokenizer.tokens()->tokAt(5));
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
            if (!scope)
                return;
            ASSERT(scope->functionOf && scope->functionOf == db->findScopeByName("Fred"));

            const Function *function = findFunctionByName("func", &db->scopeList.back());

            ASSERT(function && function->token->str() == "func");
            ASSERT(function && function->token == functionToken);
            ASSERT(function && function->hasBody() && function->isInline());
            ASSERT(function && function->functionScope == scope && scope->function == function && function->nestedIn == db->findScopeByName("Fred"));
            ASSERT(function && function->retDef == functionToken->previous());

            ASSERT(db && db->findScopeByName("Fred") && db->findScopeByName("Fred")->definedType->getFunction("func") == function);
        }
    }


    void hasInlineClassFunction_trailingReturnType() {
        GET_SYMBOL_DB("class Fred { auto func() -> int { } };")

        // 3 scopes: Global, Class, and Function
        ASSERT(db && db->scopeList.size() == 3);

        if (db) {
            const Token * const functionToken = Token::findsimplematch(tokenizer.tokens(), "func");

            const Scope *scope = findFunctionScopeByToken(db, functionToken);

            ASSERT(scope && scope->className == "func");
            if (!scope)
                return;
            ASSERT(scope->functionOf && scope->functionOf == db->findScopeByName("Fred"));

            const Function *function = findFunctionByName("func", &db->scopeList.back());

            ASSERT(function && function->token->str() == "func");
            ASSERT(function && function->token == functionToken);
            ASSERT(function && function->hasBody() && function->isInline());
            ASSERT(function && function->functionScope == scope && scope->function == function && function->nestedIn == db->findScopeByName("Fred"));
            ASSERT(function && function->retDef == functionToken->tokAt(4));

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

    void hasInlineClassOperatorTemplate() {
        GET_SYMBOL_DB("struct Fred { template<typename T> Foo & operator=(const Foo &) { return *this; } };");

        // 3 scopes: Global, Class, and Function
        ASSERT(db && db->scopeList.size() == 3);

        if (db) {
            const Token * const functionToken = Token::findsimplematch(tokenizer.tokens(), "operator=");

            const Scope *scope = findFunctionScopeByToken(db, functionToken);

            ASSERT(scope && scope->className == "operator=");
            if (!scope)
                return;
            ASSERT(scope->functionOf && scope->functionOf == db->findScopeByName("Fred"));

            const Function *function = findFunctionByName("operator=", &db->scopeList.back());

            ASSERT(function && function->token->str() == "operator=");
            ASSERT(function && function->token == functionToken);
            ASSERT(function && function->hasBody() && function->isInline());
            ASSERT(function && function->functionScope == scope && scope->function == function && function->nestedIn == db->findScopeByName("Fred"));
            ASSERT(function && function->retDef == functionToken->tokAt(-2));

            ASSERT(db && db->findScopeByName("Fred") && db->findScopeByName("Fred")->definedType->getFunction("operator=") == function);
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
            if (!scope)
                return;
            ASSERT(scope->functionOf && scope->functionOf == db->findScopeByName("Fred"));

            const Function *function = findFunctionByName("func", &db->scopeList.back());

            ASSERT(function && function->token->str() == "func");
            ASSERT(function && function->token == functionToken);
            ASSERT(function && function->hasBody() && !function->isInline());
            ASSERT(function && function->functionScope == scope && scope->function == function && function->nestedIn == db->findScopeByName("Fred"));
        }
    }

    void hasClassFunction_trailingReturnType() {
        GET_SYMBOL_DB("class Fred { auto func() -> int; }; auto Fred::func() -> int { }\n");

        // 3 scopes: Global, Class, and Function
        ASSERT(db && db->scopeList.size() == 3);

        if (db) {
            const Token * const functionToken = Token::findsimplematch(tokenizer.tokens()->linkAt(2), "func");

            const Scope *scope = findFunctionScopeByToken(db, functionToken);

            ASSERT(scope && scope->className == "func");
            if (!scope)
                return;
            ASSERT(scope->functionOf && scope->functionOf == db->findScopeByName("Fred"));

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

    void methodWithRedundantScope() {
        GET_SYMBOL_DB("class Fred { void Fred::func() {} };\n")

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

    void complexFunctionArrayPtr() {
        GET_SYMBOL_DB("int(*p1)[10]; \n"                            // pointer to array 10 of int
                      "void(*p2)(char); \n"                         // pointer to function (char) returning void
                      "int(*(*p3)(char))[10];\n"                    // pointer to function (char) returning pointer to array 10 of int
                      "float(*(*p4)(char))(long); \n"               // pointer to function (char) returning pointer to function (long) returning float
                      "short(*(*(*p5) (char))(long))(double);\n"    // pointer to function (char) returning pointer to function (long) returning pointer to function (double) returning short
                      "int(*a1[10])(void); \n"                      // array 10 of pointer to function (void) returning int
                      "float(*(*a2[10])(char))(long);\n"            // array 10 of pointer to func (char) returning pointer to func (long) returning float
                      "short(*(*(*a3[10])(char))(long))(double);\n" // array 10 of pointer to function (char) returning pointer to function (long) returning pointer to function (double) returning short
                      "::boost::rational(&r_)[9];\n"                // reference to array of ::boost::rational
                      "::boost::rational<T>(&r_)[9];");             // reference to array of ::boost::rational<T> (template!)

        ASSERT(db != nullptr);

        if (db) {
            ASSERT_EQUALS(10, db->variableList().size() - 1);
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
            ASSERT_EQUALS(1, db->variableList().size() - 1);
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
            GET_SYMBOL_DB("class Foo { Foo(); };");
            const Function* ctor = tokenizer.tokens()->tokAt(3)->function();
            ASSERT(db && ctor && ctor->type == Function::eConstructor);
            ASSERT(ctor && ctor->retDef == 0);
        }
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
            GET_SYMBOL_DB("class Foo { Foo(Bar& f); };");
            const Function* ctor = tokenizer.tokens()->tokAt(3)->function();
            ASSERT(db && ctor && ctor->type == Function::eConstructor);
            ASSERT(ctor && ctor->retDef == 0);
        }
        {
            GET_SYMBOL_DB("class Foo { Foo(Foo& f); };");
            const Function* ctor = tokenizer.tokens()->tokAt(3)->function();
            ASSERT(db && ctor && ctor->type == Function::eCopyConstructor);
            ASSERT(ctor && ctor->retDef == 0);
        }
        {
            GET_SYMBOL_DB("class Foo { Foo(const Foo &f); };");
            const Function* ctor = tokenizer.tokens()->tokAt(3)->function();
            ASSERT(db && ctor && ctor->type == Function::eCopyConstructor);
            ASSERT(ctor && ctor->retDef == 0);
        }
        {
            GET_SYMBOL_DB("template <T> class Foo { Foo(Foo<T>& f); };");
            const Function* ctor = tokenizer.tokens()->tokAt(7)->function();
            ASSERT(db && ctor && ctor->type == Function::eCopyConstructor);
            ASSERT(ctor && ctor->retDef == 0);
        }
        {
            GET_SYMBOL_DB("class Foo { Foo(Foo& f, int default = 0); };");
            const Function* ctor = tokenizer.tokens()->tokAt(3)->function();
            ASSERT(db && ctor && ctor->type == Function::eCopyConstructor);
            ASSERT(ctor && ctor->retDef == 0);
        }
        {
            GET_SYMBOL_DB("class Foo { Foo(Foo& f, char noDefault); };");
            const Function* ctor = tokenizer.tokens()->tokAt(3)->function();
            ASSERT(db && ctor && ctor->type == Function::eConstructor);
            ASSERT(ctor && ctor->retDef == 0);
        }
        {
            GET_SYMBOL_DB("class Foo { Foo(Foo&& f); };");
            const Function* ctor = tokenizer.tokens()->tokAt(3)->function();
            ASSERT(db && ctor && ctor->type == Function::eMoveConstructor);
            ASSERT(ctor && ctor->retDef == 0);
        }
        {
            GET_SYMBOL_DB("class Foo { Foo(Foo & & f, int default = 1, bool defaultToo = true); };");
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

        ASSERT(db && errout.str().empty());

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

        ASSERT(db && errout.str().empty());

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

        ASSERT(db && errout.str().empty());

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

        ASSERT(db && errout.str().empty());

        if (db) {
            const Token *f = Token::findsimplematch(tokenizer.tokens(), "f1");
            ASSERT(f && f->function() && f->function()->isStaticLocal() && f->function()->retDef->str() == "void");

            f = Token::findsimplematch(tokenizer.tokens(), "f2");
            ASSERT(f && f->function() && !f->function()->isStaticLocal() && f->function()->retDef->str() == "void");

            f = Token::findsimplematch(tokenizer.tokens(), "f3");
            ASSERT(f && f->function() && f->function()->isExtern() && f->function()->retDef->str() == "void");

            f = Token::findsimplematch(tokenizer.tokens(), "f4");
            ASSERT(f && f->function() && !f->function()->isExtern() && f->function()->retDef->str() == "void");

            f = Token::findsimplematch(tokenizer.tokens(), "f5");
            ASSERT(f && f->function() && f->function()->isExtern() && f->function()->retDef->str() == "void");

            f = Token::findsimplematch(tokenizer.tokens(), "f6");
            ASSERT(f && f->function() && !f->function()->isExtern() && f->function()->retDef->str() == "void");
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
                ASSERT(bar1->definedType->friendList.size() == 1 && bar1->definedType->friendList.front().nameEnd->str() == "Foo" && bar1->definedType->friendList.front().type == foo->definedType);
                ASSERT(bar2->definedType->friendList.size() == 1 && bar2->definedType->friendList.front().nameEnd->str() == "Foo" && bar2->definedType->friendList.front().type == foo->definedType);
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
        ASSERT_EQUALS(3, db->findScopeByName("func")->bodyStart->linenr());
    }

    void Cpp11InitInInitList() {
        GET_SYMBOL_DB("class Foo {\n"
                      "    std::vector<std::string> bar;\n"
                      "    Foo() : bar({\"a\", \"b\"})\n"
                      "    {}\n"
                      "};");
        ASSERT_EQUALS(4, db->scopeList.front().nestedList.front()->nestedList.front()->bodyStart->linenr());
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

        ASSERT(db && db->variableList().size() == 6 && db->getVariableFromVarId(1) && db->getVariableFromVarId(2) && db->getVariableFromVarId(3) && db->getVariableFromVarId(4) && db->getVariableFromVarId(5));
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

    void checkTypeStartEndToken3() {
        GET_SYMBOL_DB("void f(const char) {}");

        ASSERT(db && db->functionScopes.size()==1U);
        if (db && db->functionScopes.size()==1U) {
            const Function * const f = db->functionScopes.front()->function;
            ASSERT_EQUALS(1U, f->argCount());
            const Variable * const arg1 = f->getArgumentVar(0);
            ASSERT_EQUALS("char", arg1->typeStartToken()->str());
            ASSERT_EQUALS("char", arg1->typeEndToken()->str());
        }
    }

    void check(const char code[], bool debug = true, const char filename[] = "test.cpp") {
        // Clear the error log
        errout.str("");

        // Check..
        settings1.debugwarnings = debug;

        // Tokenize..
        Tokenizer tokenizer(&settings1, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, filename);
        tokenizer.simplifyTokenList2();

        // force symbol database creation
        tokenizer.createSymbolDatabase();

        settings1.debugwarnings = false;
    }

    void functionArgs1() {
        {
            GET_SYMBOL_DB("void f(std::vector<std::string>, const std::vector<int> & v) { }");
            ASSERT_EQUALS(1+1, db->variableList().size());
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
            ASSERT_EQUALS(1+1, db->variableList().size());
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
            GET_SYMBOL_DB("void g(int*) { }"); // unnamed pointer argument (#8052)
            const Scope* g = db->findScopeByName("g");
            ASSERT(g && g->type == Scope::eFunction && g->function && g->function->argumentList.size() == 1 && g->function->argumentList.front().nameToken() == nullptr && g->function->argumentList.front().isPointer());
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

    void functionArgs5() { // #7650
        GET_SYMBOL_DB("class ABC {};\n"
                      "class Y {\n"
                      "  enum ABC {A,B,C};\n"
                      "  void f(enum ABC abc) {}\n"
                      "};");
        ASSERT_EQUALS(true, db != nullptr);
        if (db) {
            const Token *f = Token::findsimplematch(tokenizer.tokens(), "f ( enum");
            ASSERT_EQUALS(true, f && f->function());
            if (f && f->function()) {
                const Function *func = f->function();
                ASSERT_EQUALS(true, func->argumentList.size() == 1 && func->argumentList.front().type());
                if (func->argumentList.size() == 1 && func->argumentList.front().type()) {
                    const Type * type = func->argumentList.front().type();
                    ASSERT_EQUALS(true, type->isEnumType());
                }
            }
        }
    }

    void functionArgs6() { // #7651
        GET_SYMBOL_DB("class ABC {};\n"
                      "class Y {\n"
                      "  enum ABC {A,B,C};\n"
                      "  void f(ABC abc) {}\n"
                      "};");
        ASSERT_EQUALS(true, db != nullptr);
        if (db) {
            const Token *f = Token::findsimplematch(tokenizer.tokens(), "f ( ABC");
            ASSERT_EQUALS(true, f && f->function());
            if (f && f->function()) {
                const Function *func = f->function();
                ASSERT_EQUALS(true, func->argumentList.size() == 1 && func->argumentList.front().type());
                if (func->argumentList.size() == 1 && func->argumentList.front().type()) {
                    const Type * type = func->argumentList.front().type();
                    ASSERT_EQUALS(true, type->isEnumType());
                }
            }
        }
    }

    void functionArgs7() { // #7652
        {
            GET_SYMBOL_DB("struct AB { int a; int b; };\n"
                          "int foo(struct AB *ab);\n"
                          "void bar() {\n"
                          "  struct AB ab;\n"
                          "  foo(&ab); \n"
                          "};");
            ASSERT_EQUALS(true, db != nullptr);
            if (db) {
                const Token *f = Token::findsimplematch(tokenizer.tokens(), "foo ( & ab");
                ASSERT_EQUALS(true, f && f->function());
                if (f && f->function()) {
                    const Function *func = f->function();
                    ASSERT_EQUALS(true, func->tokenDef->linenr() == 2 && func->argumentList.size() == 1 && func->argumentList.front().type());
                    if (func->argumentList.size() == 1 && func->argumentList.front().type()) {
                        const Type * type = func->argumentList.front().type();
                        ASSERT_EQUALS(true, type->classDef->linenr() == 1);
                    }
                }
            }
        }
        {
            GET_SYMBOL_DB("struct AB { int a; int b; };\n"
                          "int foo(AB *ab);\n"
                          "void bar() {\n"
                          "  struct AB ab;\n"
                          "  foo(&ab); \n"
                          "};");
            ASSERT_EQUALS(true, db != nullptr);
            if (db) {
                const Token *f = Token::findsimplematch(tokenizer.tokens(), "foo ( & ab");
                ASSERT_EQUALS(true, f && f->function());
                if (f && f->function()) {
                    const Function *func = f->function();
                    ASSERT_EQUALS(true, func->tokenDef->linenr() == 2 && func->argumentList.size() == 1 && func->argumentList.front().type());
                    if (func->argumentList.size() == 1 && func->argumentList.front().type()) {
                        const Type * type = func->argumentList.front().type();
                        ASSERT_EQUALS(true, type->classDef->linenr() == 1);
                    }
                }
            }
        }
        {
            GET_SYMBOL_DB("struct AB { int a; int b; };\n"
                          "int foo(struct AB *ab);\n"
                          "void bar() {\n"
                          "  AB ab;\n"
                          "  foo(&ab); \n"
                          "};");
            ASSERT_EQUALS(true, db != nullptr);
            if (db) {
                const Token *f = Token::findsimplematch(tokenizer.tokens(), "foo ( & ab");
                ASSERT_EQUALS(true, f && f->function());
                if (f && f->function()) {
                    const Function *func = f->function();
                    ASSERT_EQUALS(true, func->tokenDef->linenr() == 2 && func->argumentList.size() == 1 && func->argumentList.front().type());
                    if (func->argumentList.size() == 1 && func->argumentList.front().type()) {
                        const Type * type = func->argumentList.front().type();
                        ASSERT_EQUALS(true, type->classDef->linenr() == 1);
                    }
                }
            }
        }
        {
            GET_SYMBOL_DB("struct AB { int a; int b; };\n"
                          "int foo(AB *ab);\n"
                          "void bar() {\n"
                          "  AB ab;\n"
                          "  foo(&ab); \n"
                          "};");
            ASSERT_EQUALS(true, db != nullptr);
            if (db) {
                const Token *f = Token::findsimplematch(tokenizer.tokens(), "foo ( & ab");
                ASSERT_EQUALS(true, f && f->function());
                if (f && f->function()) {
                    const Function *func = f->function();
                    ASSERT_EQUALS(true, func->tokenDef->linenr() == 2 && func->argumentList.size() == 1 && func->argumentList.front().type());
                    if (func->argumentList.size() == 1 && func->argumentList.front().type()) {
                        const Type * type = func->argumentList.front().type();
                        ASSERT_EQUALS(true, type->classDef->linenr() == 1);
                    }
                }
            }
        }
    }

    void functionArgs8() { // #7653
        GET_SYMBOL_DB("struct A { int i; };\n"
                      "struct B { double d; };\n"
                      "int    foo(struct A a);\n"
                      "double foo(struct B b);\n"
                      "void bar() {\n"
                      "  struct B b;\n"
                      "  foo(b);\n"
                      "}");
        ASSERT_EQUALS(true, db != nullptr);
        if (db) {
            const Token *f = Token::findsimplematch(tokenizer.tokens(), "foo ( b");
            ASSERT_EQUALS(true, f && f->function());
            if (f && f->function()) {
                const Function *func = f->function();
                ASSERT_EQUALS(true, func->tokenDef->linenr() == 4 && func->argumentList.size() == 1 && func->argumentList.front().type());
                if (func->argumentList.size() == 1 && func->argumentList.front().type()) {
                    const Type * type = func->argumentList.front().type();
                    ASSERT_EQUALS(true, type->isStructType());
                }
            }
        }
    }

    void functionArgs9() { // #7657
        GET_SYMBOL_DB("struct A {\n"
                      "  struct B {\n"
                      "    enum C { };\n"
                      "  };\n"
                      "};\n"
                      "void foo(A::B::C c) { }");
        ASSERT_EQUALS(true, db != nullptr);
        if (db) {
            const Token *f = Token::findsimplematch(tokenizer.tokens(), "foo (");
            ASSERT_EQUALS(true, f && f->function());
            if (f && f->function()) {
                const Function *func = f->function();
                ASSERT_EQUALS(true, func->argumentList.size() == 1 && func->argumentList.front().type());
                if (func->argumentList.size() == 1 && func->argumentList.front().type()) {
                    const Type * type = func->argumentList.front().type();
                    ASSERT_EQUALS(true, type->isEnumType());
                }
            }
        }
    }

    void functionArgs10() {
        GET_SYMBOL_DB("class Fred {\n"
                      "public:\n"
                      "  Fred(Whitespace = PRESERVE_WHITESPACE);\n"
                      "};\n"
                      "Fred::Fred(Whitespace whitespace) { }");
        ASSERT_EQUALS(true, db != nullptr);
        if (db) {
            ASSERT_EQUALS(3, db->scopeList.size());
            if (db->scopeList.size() == 3) {
                std::list<Scope>::const_iterator scope = db->scopeList.begin();
                ++scope;
                ASSERT_EQUALS((unsigned int)Scope::eClass, (unsigned int)scope->type);
                ASSERT_EQUALS(1, scope->functionList.size());
                ASSERT(scope->functionList.begin()->functionScope != nullptr);
                if (scope->functionList.begin()->functionScope) {
                    const Scope * functionScope = scope->functionList.begin()->functionScope;
                    ++scope;
                    ASSERT(functionScope == &*scope);
                }
            }
        }
    }

    void functionArgs11() {
        GET_SYMBOL_DB("class Fred {\n"
                      "public:\n"
                      "  void foo(char a[16]);\n"
                      "};\n"
                      "void Fred::foo(char b[16]) { }");
        ASSERT_EQUALS(true, db != nullptr);
        if (db) {
            ASSERT_EQUALS(3, db->scopeList.size());
            if (db->scopeList.size() == 3) {
                std::list<Scope>::const_iterator scope = db->scopeList.begin();
                ++scope;
                ASSERT_EQUALS((unsigned int)Scope::eClass, (unsigned int)scope->type);
                ASSERT_EQUALS(1, scope->functionList.size());
                ASSERT(scope->functionList.begin()->functionScope != nullptr);
                if (scope->functionList.begin()->functionScope) {
                    const Scope * functionScope = scope->functionList.begin()->functionScope;
                    ++scope;
                    ASSERT(functionScope == &*scope);
                }
            }
        }
    }

    void functionArgs12() { // #7661
        GET_SYMBOL_DB("struct A {\n"
                      "    enum E { };\n"
                      "    int a[10];\n"
                      "};\n"
                      "struct B : public A {\n"
                      "    void foo(B::E e) { }\n"
                      "};");

        ASSERT_EQUALS(true, db != nullptr);
        if (db) {
            const Token *f = Token::findsimplematch(tokenizer.tokens(), "foo (");
            ASSERT_EQUALS(true, f && f->function());
            if (f && f->function()) {
                const Function *func = f->function();
                ASSERT_EQUALS(true, func->argumentList.size() == 1 && func->argumentList.front().type());
                if (func->argumentList.size() == 1 && func->argumentList.front().type()) {
                    const Type * type = func->argumentList.front().type();
                    ASSERT_EQUALS(true, type->isEnumType());
                }
            }
        }
    }

    void functionArgs13() { // #7697
        GET_SYMBOL_DB("struct A {\n"
                      "    enum E { };\n"
                      "    struct S { };\n"
                      "};\n"
                      "struct B : public A {\n"
                      "    B(E e);\n"
                      "    B(S s);\n"
                      "};\n"
                      "B::B(A::E e) { }\n"
                      "B::B(A::S s) { }");

        ASSERT_EQUALS(true, db != nullptr);
        if (db) {
            const Token *f = Token::findsimplematch(tokenizer.tokens(), "B ( A :: E");
            ASSERT_EQUALS(true, f && f->function());
            if (f && f->function()) {
                const Function *func = f->function();
                ASSERT_EQUALS(true, func->argumentList.size() == 1 && func->argumentList.front().type());
                if (func->argumentList.size() == 1 && func->argumentList.front().type()) {
                    const Type * type = func->argumentList.front().type();
                    ASSERT_EQUALS(true, type->isEnumType() && type->name() == "E");
                }
            }
            f = Token::findsimplematch(tokenizer.tokens(), "B ( A :: S");
            ASSERT_EQUALS(true, f && f->function());
            if (f && f->function()) {
                const Function *func = f->function();
                ASSERT_EQUALS(true, func->argumentList.size() == 1 && func->argumentList.front().type());
                if (func->argumentList.size() == 1 && func->argumentList.front().type()) {
                    const Type * type = func->argumentList.front().type();
                    ASSERT_EQUALS(true, type->isStructType() && type->name() == "S");
                }
            }
        }
    }

    void functionImplicitlyVirtual() {
        GET_SYMBOL_DB("class base { virtual void f(); };\n"
                      "class derived : base { void f(); };\n"
                      "void derived::f() {}");
        ASSERT(db != nullptr);
        if (!db)
            return;
        ASSERT_EQUALS(4, db->scopeList.size());
        const Function *function = db->scopeList.back().function;
        ASSERT_EQUALS(true, function && function->isImplicitlyVirtual(false));
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

    void namespaces4() { // #4698 - type lookup
        GET_SYMBOL_DB("struct A { int a; };\n"
                      "namespace fred { struct A {}; }\n"
                      "fred::A fredA;");
        const Variable *fredA = db->getVariableFromVarId(2U);
        ASSERT_EQUALS("fredA", fredA->name());
        const Type *fredAType = fredA->type();
        ASSERT_EQUALS(2U, fredAType->classDef->linenr());
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
        ASSERT_EQUALS("", errout.str());
        ASSERT(db && db->variableList().size() == 5); // index 0 + 4 variables
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

        check("main(int argc, char *argv[]) { }", true, "test.c");
        ASSERT_EQUALS("[test.c:1]: (debug) SymbolDatabase::isFunction found C function 'main' without a return type.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:1]: (debug) Executable scope 'testing' with unknown function.\n", errout.str());
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

    void symboldatabase17() {
        // ticket #2657 - segmentation fault
        check("return f(){}");

        ASSERT_EQUALS("", errout.str());
    }

    void symboldatabase19() {
        // ticket #2991 - segmentation fault
        check("::y(){x}");

        ASSERT_EQUALS("[test.cpp:1]: (debug) Executable scope 'y' with unknown function.\n", errout.str());
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

    // ticket 3437 (segmentation fault)
    void symboldatabase22() {
        check("template <class C> struct A {};\n"
              "A<int> a;\n");
        ASSERT_EQUALS("", errout.str());
    }

    // ticket 3435 (std::vector)
    void symboldatabase23() {
        GET_SYMBOL_DB("class A { std::vector<int*> ints; };");
        ASSERT_EQUALS(2U, db->scopeList.size());
        const Scope &scope = db->scopeList.back();
        ASSERT_EQUALS(1U, scope.varlist.size());
        const Variable &var = scope.varlist.front();
        ASSERT_EQUALS(std::string("ints"), var.name());
        ASSERT_EQUALS(true, var.isClass());
    }

    // ticket 3508 (constructor, destructor)
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

    // ticket #3561 (throw C++)
    void symboldatabase25() {
        const char str[] = "int main() {\n"
                           "    foo bar;\n"
                           "    throw bar;\n"
                           "}";
        GET_SYMBOL_DB(str);
        ASSERT_EQUALS("", errout.str());
        ASSERT(db && db->variableList().size() == 2); // index 0 + 1 variable
    }

    // ticket #3561 (throw C)
    void symboldatabase26() {
        const char str[] = "int main() {\n"
                           "    throw bar;\n"
                           "}";
        GET_SYMBOL_DB_C(str);
        ASSERT_EQUALS("", errout.str());
        ASSERT(db && db->variableList().size() == 2); // index 0 + 1 variable
    }

    // ticket #3543 (segmentation fault)
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

    // ticket #4442 (segmentation fault)
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
        if (!db || db->typeList.size() != 3)
            return;
        std::list<Type>::const_iterator i = db->typeList.begin();
        const Type* Fred = &(*i++);
        const Type* Wilma = &(*i++);
        const Type* Barney = &(*i++);
        ASSERT(Fred && Fred->classDef && Fred->classScope && Fred->enclosingScope && Fred->name() == "Fred");
        ASSERT(Wilma && Wilma->classDef && Wilma->classScope && Wilma->enclosingScope && Wilma->name() == "Wilma");
        ASSERT(Barney && Barney->classDef && Barney->classScope && Barney->enclosingScope && Barney->name() == "Barney");
        ASSERT(db->variableList().size() == 5);
        if (db->variableList().size() != 5)
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
        ASSERT(db != nullptr);
        const Scope * const fscope = db ? db->findScopeByName("f") : nullptr;
        ASSERT(fscope != nullptr);
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
        ASSERT_EQUALS(4U, db->variableList().size() - 1);
        ASSERT_EQUALS(2U, db->scopeList.size());
        for (std::size_t i = 1U; i < db->variableList().size(); i++)
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
        ASSERT_EQUALS(4U, db->variableList().size() - 1);
        for (std::size_t i = 1U; i < db->variableList().size(); i++)
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

    void symboldatabase52() { // #6581
        GET_SYMBOL_DB("void foo() {\n"
                      "    int i = 0;\n"
                      "    S s{ { i }, 0 };\n"
                      "}");

        ASSERT(db != nullptr);
        if (db) {
            ASSERT_EQUALS(2, db->scopeList.size());
            ASSERT_EQUALS(2, db->variableList().size()-1);
            ASSERT(db->getVariableFromVarId(1) != nullptr);
            ASSERT(db->getVariableFromVarId(2) != nullptr);
        }
    }

    void symboldatabase53() { // #7124
        GET_SYMBOL_DB("int32_t x;"
                      "std::int32_t y;");

        ASSERT(db != nullptr);
        if (db) {
            ASSERT(db->getVariableFromVarId(1) != nullptr);
            ASSERT(db->getVariableFromVarId(2) != nullptr);
            ASSERT_EQUALS(false, db->getVariableFromVarId(1)->isClass());
            ASSERT_EQUALS(false, db->getVariableFromVarId(2)->isClass());
        }
    }

    void symboldatabase54() { // #7343
        GET_SYMBOL_DB("class A {\n"
                      "  void getReg() const override {\n"
                      "    assert(Kind == k_ShiftExtend);\n"
                      "  }\n"
                      "};");

        ASSERT(db != nullptr);
        if (db) {
            ASSERT_EQUALS(1U, db->functionScopes.size());
            ASSERT_EQUALS("getReg", db->functionScopes.front()->className);
            ASSERT_EQUALS(true, db->functionScopes.front()->function->hasOverrideSpecifier());
        }
    }

    void symboldatabase55() { // #7767
        GET_SYMBOL_DB("PRIVATE S32 testfunc(void) {\n"
                      "    return 0;\n"
                      "}");

        ASSERT(db != nullptr);
        if (db) {
            ASSERT_EQUALS(1U, db->functionScopes.size());
            ASSERT_EQUALS("testfunc", db->functionScopes.front()->className);
        }
    }

    void symboldatabase56() { // #7909
        {
            GET_SYMBOL_DB("class Class {\n"
                          "    class NestedClass {\n"
                          "    public:\n"
                          "        virtual void f();\n"
                          "    };\n"
                          "    friend void NestedClass::f();\n"
                          "}");

            ASSERT(db != nullptr);
            if (db) {
                ASSERT_EQUALS(0U, db->functionScopes.size());
                ASSERT(db->scopeList.back().type == Scope::eClass && db->scopeList.back().className == "NestedClass");
                ASSERT(db->scopeList.back().functionList.size() == 1U && !db->scopeList.back().functionList.front().hasBody());
            }
        }
        {
            GET_SYMBOL_DB("class Class {\n"
                          "    friend void f1();\n"
                          "    friend void f2() { }\n"
                          "}");

            ASSERT(db != nullptr);
            if (db) {
                ASSERT_EQUALS(1U, db->functionScopes.size());
                ASSERT(db->scopeList.back().type == Scope::eFunction && db->scopeList.back().className == "f2");
                ASSERT(db->scopeList.back().function && db->scopeList.back().function->hasBody());
            }
        }
        {
            GET_SYMBOL_DB_C("friend f1();\n"
                            "friend f2() { }\n");

            ASSERT(db != nullptr);
            if (db) {
                ASSERT_EQUALS(2U, db->scopeList.size());
                ASSERT_EQUALS(2U, db->scopeList.begin()->functionList.size());
            }
        }
    }

    void symboldatabase57() {
        GET_SYMBOL_DB("int bar(bool b)\n"
                      "{\n"
                      "    if(b)\n"
                      "         return 1;\n"
                      "    else\n"
                      "         return 1;\n"
                      "}");
        ASSERT(db != nullptr);
        if (db) {
            ASSERT(db->scopeList.size() == 4U);
            if (db->scopeList.size() == 4U) {
                std::list<Scope>::const_iterator it = db->scopeList.begin();
                ASSERT(it->type == Scope::eGlobal);
                ASSERT((++it)->type == Scope::eFunction);
                ASSERT((++it)->type == Scope::eIf);
                ASSERT((++it)->type == Scope::eElse);
            }
        }
    }

    void symboldatabase58() { // #6985 (using namespace type lookup)
        GET_SYMBOL_DB("namespace N2\n"
                      "{\n"
                      "class B { };\n"
                      "}\n"
                      "using namespace N2;\n"
                      "class C {\n"
                      "    class A : public B\n"
                      "    {\n"
                      "    };\n"
                      "};");
        ASSERT(db != nullptr);
        if (db) {
            ASSERT(db->typeList.size() == 3U);
            if (db->typeList.size() == 3U) {
                std::list<Type>::const_iterator it = db->typeList.begin();
                const Type * classB = &(*it);
                const Type * classC = &(*(++it));
                const Type * classA = &(*(++it));
                ASSERT(classA->name() == "A" && classB->name() == "B" && classC->name() == "C");
                if (classA->name() == "A" && classB->name() == "B" && classC->name() == "C") {
                    ASSERT(classA->derivedFrom.size() == 1U);
                    if (classA->derivedFrom.size() == 1) {
                        ASSERT(classA->derivedFrom[0].type != nullptr);
                        if (classA->derivedFrom[0].type != nullptr) {
                            ASSERT(classA->derivedFrom[0].type == classB);
                        }
                    }
                }
            }
        }
    }

    void symboldatabase59() { // #8465
        GET_SYMBOL_DB("struct A::B ab[10];\n"
                      "void f() {}");
        ASSERT(db != nullptr);
        ASSERT(db && db->scopeList.size() == 2);
    }

    void symboldatabase60() { // #8470
        GET_SYMBOL_DB("struct A::someType A::bar() { return 0; }");
        ASSERT(db != nullptr);
        ASSERT(db && db->scopeList.size() == 2);
    }

    void symboldatabase61() {
        GET_SYMBOL_DB("struct Fred {\n"
                      "    struct Info { };\n"
                      "};\n"
                      "void foo() {\n"
                      "    struct Fred::Info* info;\n"
                      "    info = new (nothrow) struct Fred::Info();\n"
                      "    info = new struct Fred::Info();\n"
                      "    memset(info, 0, sizeof(struct Fred::Info));\n"
                      "}");

        ASSERT(db != nullptr);
        ASSERT(db && db->scopeList.size() == 4);
    }

    void symboldatabase62() {
        GET_SYMBOL_DB("struct A {\n"
                      "public:\n"
                      "    struct X { int a; };\n"
                      "    void Foo(const std::vector<struct X> &includes);\n"
                      "};\n"
                      "void A::Foo(const std::vector<struct A::X> &includes) {\n"
                      "    for (std::vector<struct A::X>::const_iterator it = includes.begin(); it != includes.end(); ++it) {\n"
                      "        const struct A::X currentIncList = *it;\n"
                      "    }\n"
                      "}");
        ASSERT(db != nullptr);
        ASSERT(db && db->scopeList.size() == 5);
    }

    void symboldatabase63() {
        {
            GET_SYMBOL_DB("template class T<int> ; void foo() { }");
            ASSERT(db != nullptr);
            ASSERT(db && db->scopeList.size() == 2);
        }
        {
            GET_SYMBOL_DB("template struct T<int> ; void foo() { }");
            ASSERT(db != nullptr);
            ASSERT(db && db->scopeList.size() == 2);
        }
    }

    void symboldatabase64() {
        {
            GET_SYMBOL_DB("class Fred { struct impl; };\n"
                          "struct Fred::impl {\n"
                          "    impl() { }\n"
                          "    ~impl() { }\n"
                          "    impl(const impl &) { }\n"
                          "    void foo(const impl &, const impl &) const { }\n"
                          "};");

            ASSERT(db != nullptr);
            ASSERT(db && db->scopeList.size() == 7);
            ASSERT(db && db->classAndStructScopes.size() == 2);
            ASSERT(db && db->typeList.size() == 2);
            ASSERT(db && db->functionScopes.size() == 4);

            const Token * functionToken = Token::findsimplematch(tokenizer.tokens(), "impl ( ) { }");
            ASSERT(db && functionToken && functionToken->function() &&
                   functionToken->function()->functionScope &&
                   functionToken->function()->tokenDef->linenr() == 3 &&
                   functionToken->function()->token->linenr() == 3);

            functionToken = Token::findsimplematch(tokenizer.tokens(), "~ impl ( ) { }");
            ASSERT(db && functionToken && functionToken->next()->function() &&
                   functionToken->next()->function()->functionScope &&
                   functionToken->next()->function()->tokenDef->linenr() == 4 &&
                   functionToken->next()->function()->token->linenr() == 4);

            functionToken = Token::findsimplematch(tokenizer.tokens(), "impl ( const impl & ) { }");
            ASSERT(db && functionToken && functionToken->function() &&
                   functionToken->function()->functionScope &&
                   functionToken->function()->tokenDef->linenr() == 5 &&
                   functionToken->function()->token->linenr() == 5);

            functionToken = Token::findsimplematch(tokenizer.tokens(), "foo ( const impl & , const impl & ) const { }");
            ASSERT(db && functionToken && functionToken->function() &&
                   functionToken->function()->functionScope &&
                   functionToken->function()->tokenDef->linenr() == 6 &&
                   functionToken->function()->token->linenr() == 6);
        }
        {
            GET_SYMBOL_DB("class Fred { struct impl; };\n"
                          "struct Fred::impl {\n"
                          "    impl();\n"
                          "    ~impl();\n"
                          "    impl(const impl &);\n"
                          "    void foo(const impl &, const impl &) const;\n"
                          "};\n"
                          "Fred::impl::impl() { }\n"
                          "Fred::impl::~impl() { }\n"
                          "Fred::impl::impl(const Fred::impl &) { }\n"
                          "void Fred::impl::foo(const Fred::impl &, const Fred::impl &) const { }");

            ASSERT(db != nullptr);
            ASSERT(db && db->scopeList.size() == 7);
            ASSERT(db && db->classAndStructScopes.size() == 2);
            ASSERT(db && db->typeList.size() == 2);
            ASSERT(db && db->functionScopes.size() == 4);

            const Token * functionToken = Token::findsimplematch(tokenizer.tokens(), "impl ( ) { }");
            ASSERT(db && functionToken && functionToken->function() &&
                   functionToken->function()->functionScope &&
                   functionToken->function()->tokenDef->linenr() == 3 &&
                   functionToken->function()->token->linenr() == 8);

            functionToken = Token::findsimplematch(tokenizer.tokens(), "~ impl ( ) { }");
            ASSERT(db && functionToken && functionToken->next()->function() &&
                   functionToken->next()->function()->functionScope &&
                   functionToken->next()->function()->tokenDef->linenr() == 4 &&
                   functionToken->next()->function()->token->linenr() == 9);

            functionToken = Token::findsimplematch(tokenizer.tokens(), "impl ( const Fred :: impl & ) { }");
            ASSERT(db && functionToken && functionToken->function() &&
                   functionToken->function()->functionScope &&
                   functionToken->function()->tokenDef->linenr() == 5 &&
                   functionToken->function()->token->linenr() == 10);

            functionToken = Token::findsimplematch(tokenizer.tokens(), "foo ( const Fred :: impl & , const Fred :: impl & ) const { }");
            ASSERT(db && functionToken && functionToken->function() &&
                   functionToken->function()->functionScope &&
                   functionToken->function()->tokenDef->linenr() == 6 &&
                   functionToken->function()->token->linenr() == 11);
        }
        {
            GET_SYMBOL_DB("namespace NS {\n"
                          "    class Fred { struct impl; };\n"
                          "    struct Fred::impl {\n"
                          "        impl() { }\n"
                          "        ~impl() { }\n"
                          "        impl(const impl &) { }\n"
                          "        void foo(const impl &, const impl &) const { }\n"
                          "    };\n"
                          "}");

            ASSERT(db != nullptr);
            ASSERT(db && db->scopeList.size() == 8);
            ASSERT(db && db->classAndStructScopes.size() == 2);
            ASSERT(db && db->typeList.size() == 2);
            ASSERT(db && db->functionScopes.size() == 4);

            const Token * functionToken = Token::findsimplematch(tokenizer.tokens(), "impl ( ) { }");
            ASSERT(db && functionToken && functionToken->function() &&
                   functionToken->function()->functionScope &&
                   functionToken->function()->tokenDef->linenr() == 4 &&
                   functionToken->function()->token->linenr() == 4);

            functionToken = Token::findsimplematch(tokenizer.tokens(), "~ impl ( ) { }");
            ASSERT(db && functionToken && functionToken->next()->function() &&
                   functionToken->next()->function()->functionScope &&
                   functionToken->next()->function()->tokenDef->linenr() == 5 &&
                   functionToken->next()->function()->token->linenr() == 5);

            functionToken = Token::findsimplematch(tokenizer.tokens(), "impl ( const impl & ) { }");
            ASSERT(db && functionToken && functionToken->function() &&
                   functionToken->function()->functionScope &&
                   functionToken->function()->tokenDef->linenr() == 6 &&
                   functionToken->function()->token->linenr() == 6);

            functionToken = Token::findsimplematch(tokenizer.tokens(), "foo ( const impl & , const impl & ) const { }");
            ASSERT(db && functionToken && functionToken->function() &&
                   functionToken->function()->functionScope &&
                   functionToken->function()->tokenDef->linenr() == 7 &&
                   functionToken->function()->token->linenr() == 7);
        }
        {
            GET_SYMBOL_DB("namespace NS {\n"
                          "    class Fred { struct impl; };\n"
                          "    struct Fred::impl {\n"
                          "        impl();\n"
                          "        ~impl();\n"
                          "        impl(const impl &);\n"
                          "        void foo(const impl &, const impl &) const;\n"
                          "    };\n"
                          "    Fred::impl::impl() { }\n"
                          "    Fred::impl::~impl() { }\n"
                          "    Fred::impl::impl(const Fred::impl &) { }\n"
                          "    void Fred::impl::foo(const Fred::impl &, const Fred::impl &) const { }\n"
                          "}");

            ASSERT(db != nullptr);
            ASSERT(db && db->scopeList.size() == 8);
            ASSERT(db && db->classAndStructScopes.size() == 2);
            ASSERT(db && db->typeList.size() == 2);
            ASSERT(db && db->functionScopes.size() == 4);

            const Token * functionToken = Token::findsimplematch(tokenizer.tokens(), "impl ( ) { }");
            ASSERT(db && functionToken && functionToken->function() &&
                   functionToken->function()->functionScope &&
                   functionToken->function()->tokenDef->linenr() == 4 &&
                   functionToken->function()->token->linenr() == 9);

            functionToken = Token::findsimplematch(tokenizer.tokens(), "~ impl ( ) { }");
            ASSERT(db && functionToken && functionToken->next()->function() &&
                   functionToken->next()->function()->functionScope &&
                   functionToken->next()->function()->tokenDef->linenr() == 5 &&
                   functionToken->next()->function()->token->linenr() == 10);

            functionToken = Token::findsimplematch(tokenizer.tokens(), "impl ( const Fred :: impl & ) { }");
            ASSERT(db && functionToken && functionToken->function() &&
                   functionToken->function()->functionScope &&
                   functionToken->function()->tokenDef->linenr() == 6 &&
                   functionToken->function()->token->linenr() == 11);

            functionToken = Token::findsimplematch(tokenizer.tokens(), "foo ( const Fred :: impl & , const Fred :: impl & ) const { }");
            ASSERT(db && functionToken && functionToken->function() &&
                   functionToken->function()->functionScope &&
                   functionToken->function()->tokenDef->linenr() == 7 &&
                   functionToken->function()->token->linenr() == 12);
        }
        {
            GET_SYMBOL_DB("namespace NS {\n"
                          "    class Fred { struct impl; };\n"
                          "    struct Fred::impl {\n"
                          "        impl();\n"
                          "        ~impl();\n"
                          "        impl(const impl &);\n"
                          "        void foo(const impl &, const impl &) const;\n"
                          "    };\n"
                          "}\n"
                          "NS::Fred::impl::impl() { }\n"
                          "NS::Fred::impl::~impl() { }\n"
                          "NS::Fred::impl::impl(const NS::Fred::impl &) { }\n"
                          "void NS::Fred::impl::foo(const NS::Fred::impl &, const NS::Fred::impl &) const { }");

            ASSERT(db != nullptr);
            ASSERT(db && db->scopeList.size() == 8);
            ASSERT(db && db->classAndStructScopes.size() == 2);
            ASSERT(db && db->typeList.size() == 2);
            ASSERT(db && db->functionScopes.size() == 4);

            const Token * functionToken = Token::findsimplematch(tokenizer.tokens(), "impl ( ) { }");
            ASSERT(db && functionToken && functionToken->function() &&
                   functionToken->function()->functionScope &&
                   functionToken->function()->tokenDef->linenr() == 4 &&
                   functionToken->function()->token->linenr() == 10);

            functionToken = Token::findsimplematch(tokenizer.tokens(), "~ impl ( ) { }");
            ASSERT(db && functionToken && functionToken->next()->function() &&
                   functionToken->next()->function()->functionScope &&
                   functionToken->next()->function()->tokenDef->linenr() == 5 &&
                   functionToken->next()->function()->token->linenr() == 11);

            functionToken = Token::findsimplematch(tokenizer.tokens(), "impl ( const NS :: Fred :: impl & ) { }");
            ASSERT(db && functionToken && functionToken->function() &&
                   functionToken->function()->functionScope &&
                   functionToken->function()->tokenDef->linenr() == 6 &&
                   functionToken->function()->token->linenr() == 12);

            functionToken = Token::findsimplematch(tokenizer.tokens(), "foo ( const NS :: Fred :: impl & , const NS :: Fred :: impl & ) const { }");
            ASSERT(db && functionToken && functionToken->function() &&
                   functionToken->function()->functionScope &&
                   functionToken->function()->tokenDef->linenr() == 7 &&
                   functionToken->function()->token->linenr() == 13);
        }
        {
            GET_SYMBOL_DB("namespace NS {\n"
                          "    class Fred { struct impl; };\n"
                          "}\n"
                          "struct NS::Fred::impl {\n"
                          "    impl() { }\n"
                          "    ~impl() { }\n"
                          "    impl(const impl &) { }\n"
                          "    void foo(const impl &, const impl &) const { }\n"
                          "};");

            ASSERT(db != nullptr);
            ASSERT(db && db->scopeList.size() == 8);
            ASSERT(db && db->classAndStructScopes.size() == 2);
            ASSERT(db && db->typeList.size() == 2);
            ASSERT(db && db->functionScopes.size() == 4);

            const Token * functionToken = Token::findsimplematch(tokenizer.tokens(), "impl ( ) { }");
            ASSERT(db && functionToken && functionToken->function() &&
                   functionToken->function()->functionScope &&
                   functionToken->function()->tokenDef->linenr() == 5 &&
                   functionToken->function()->token->linenr() == 5);

            functionToken = Token::findsimplematch(tokenizer.tokens(), "~ impl ( ) { }");
            ASSERT(db && functionToken && functionToken->next()->function() &&
                   functionToken->next()->function()->functionScope &&
                   functionToken->next()->function()->tokenDef->linenr() == 6 &&
                   functionToken->next()->function()->token->linenr() == 6);

            functionToken = Token::findsimplematch(tokenizer.tokens(), "impl ( const impl & ) { }");
            ASSERT(db && functionToken && functionToken->function() &&
                   functionToken->function()->functionScope &&
                   functionToken->function()->tokenDef->linenr() == 7 &&
                   functionToken->function()->token->linenr() == 7);

            functionToken = Token::findsimplematch(tokenizer.tokens(), "foo ( const impl & , const impl & ) const { }");
            ASSERT(db && functionToken && functionToken->function() &&
                   functionToken->function()->functionScope &&
                   functionToken->function()->tokenDef->linenr() == 8 &&
                   functionToken->function()->token->linenr() == 8);
        }
        {
            GET_SYMBOL_DB("namespace NS {\n"
                          "    class Fred { struct impl; };\n"
                          "}\n"
                          "struct NS::Fred::impl {\n"
                          "    impl();\n"
                          "    ~impl();\n"
                          "    impl(const impl &);\n"
                          "    void foo(const impl &, const impl &) const;\n"
                          "};\n"
                          "NS::Fred::impl::impl() { }\n"
                          "NS::Fred::impl::~impl() { }\n"
                          "NS::Fred::impl::impl(const NS::Fred::impl &) { }\n"
                          "void NS::Fred::impl::foo(const NS::Fred::impl &, const NS::Fred::impl &) const { }");

            ASSERT(db != nullptr);
            ASSERT(db && db->scopeList.size() == 8);
            ASSERT(db && db->classAndStructScopes.size() == 2);
            ASSERT(db && db->typeList.size() == 2);
            ASSERT(db && db->functionScopes.size() == 4);

            const Token * functionToken = Token::findsimplematch(tokenizer.tokens(), "impl ( ) { }");
            ASSERT(db && functionToken && functionToken->function() &&
                   functionToken->function()->functionScope &&
                   functionToken->function()->tokenDef->linenr() == 5 &&
                   functionToken->function()->token->linenr() == 10);

            functionToken = Token::findsimplematch(tokenizer.tokens(), "~ impl ( ) { }");
            ASSERT(db && functionToken && functionToken->next()->function() &&
                   functionToken->next()->function()->functionScope &&
                   functionToken->next()->function()->tokenDef->linenr() == 6 &&
                   functionToken->next()->function()->token->linenr() == 11);

            functionToken = Token::findsimplematch(tokenizer.tokens(), "impl ( const NS :: Fred :: impl & ) { }");
            ASSERT(db && functionToken && functionToken->function() &&
                   functionToken->function()->functionScope &&
                   functionToken->function()->tokenDef->linenr() == 7 &&
                   functionToken->function()->token->linenr() == 12);

            functionToken = Token::findsimplematch(tokenizer.tokens(), "foo ( const NS :: Fred :: impl & , const NS :: Fred :: impl & ) const { }");
            ASSERT(db && functionToken && functionToken->function() &&
                   functionToken->function()->functionScope &&
                   functionToken->function()->tokenDef->linenr() == 8 &&
                   functionToken->function()->token->linenr() == 13);
        }
        {
            GET_SYMBOL_DB("namespace NS {\n"
                          "    class Fred { struct impl; };\n"
                          "}\n"
                          "struct NS::Fred::impl {\n"
                          "    impl();\n"
                          "    ~impl();\n"
                          "    impl(const impl &);\n"
                          "    void foo(const impl &, const impl &) const;\n"
                          "};\n"
                          "namespace NS {\n"
                          "    Fred::impl::impl() { }\n"
                          "    Fred::impl::~impl() { }\n"
                          "    Fred::impl::impl(const Fred::impl &) { }\n"
                          "    void Fred::impl::foo(const Fred::impl &, const Fred::impl &) const { }\n"
                          "}");

            ASSERT(db != nullptr);
            ASSERT(db && db->scopeList.size() == 8);
            ASSERT(db && db->classAndStructScopes.size() == 2);
            ASSERT(db && db->typeList.size() == 2);
            ASSERT(db && db->functionScopes.size() == 4);

            const Token * functionToken = Token::findsimplematch(tokenizer.tokens(), "impl ( ) { }");
            ASSERT(db && functionToken && functionToken->function() &&
                   functionToken->function()->functionScope &&
                   functionToken->function()->tokenDef->linenr() == 5 &&
                   functionToken->function()->token->linenr() == 11);

            functionToken = Token::findsimplematch(tokenizer.tokens(), "~ impl ( ) { }");
            ASSERT(db && functionToken && functionToken->next()->function() &&
                   functionToken->next()->function()->functionScope &&
                   functionToken->next()->function()->tokenDef->linenr() == 6 &&
                   functionToken->next()->function()->token->linenr() == 12);

            functionToken = Token::findsimplematch(tokenizer.tokens(), "impl ( const Fred :: impl & ) { }");
            ASSERT(db && functionToken && functionToken->function() &&
                   functionToken->function()->functionScope &&
                   functionToken->function()->tokenDef->linenr() == 7 &&
                   functionToken->function()->token->linenr() == 13);

            functionToken = Token::findsimplematch(tokenizer.tokens(), "foo ( const Fred :: impl & , const Fred :: impl & ) const { }");
            ASSERT(db && functionToken && functionToken->function() &&
                   functionToken->function()->functionScope &&
                   functionToken->function()->tokenDef->linenr() == 8 &&
                   functionToken->function()->token->linenr() == 14);
        }
        {
            GET_SYMBOL_DB("namespace NS {\n"
                          "    class Fred { struct impl; };\n"
                          "}\n"
                          "struct NS::Fred::impl {\n"
                          "    impl();\n"
                          "    ~impl();\n"
                          "    impl(const impl &);\n"
                          "    void foo(const impl &, const impl &) const;\n"
                          "};\n"
                          "using namespace NS;\n"
                          "Fred::impl::impl() { }\n"
                          "Fred::impl::~impl() { }\n"
                          "Fred::impl::impl(const Fred::impl &) { }\n"
                          "void Fred::impl::foo(const Fred::impl &, const Fred::impl &) const { }");

            ASSERT(db != nullptr);
            ASSERT(db && db->scopeList.size() == 8);
            ASSERT(db && db->classAndStructScopes.size() == 2);
            ASSERT(db && db->typeList.size() == 2);
            ASSERT(db && db->functionScopes.size() == 4);

            const Token * functionToken = Token::findsimplematch(tokenizer.tokens(), "impl ( ) { }");
            ASSERT(db && functionToken && functionToken->function() &&
                   functionToken->function()->functionScope &&
                   functionToken->function()->tokenDef->linenr() == 5 &&
                   functionToken->function()->token->linenr() == 11);

            functionToken = Token::findsimplematch(tokenizer.tokens(), "~ impl ( ) { }");
            ASSERT(db && functionToken && functionToken->next()->function() &&
                   functionToken->next()->function()->functionScope &&
                   functionToken->next()->function()->tokenDef->linenr() == 6 &&
                   functionToken->next()->function()->token->linenr() == 12);

            functionToken = Token::findsimplematch(tokenizer.tokens(), "impl ( const Fred :: impl & ) { }");
            ASSERT(db && functionToken && functionToken->function() &&
                   functionToken->function()->functionScope &&
                   functionToken->function()->tokenDef->linenr() == 7 &&
                   functionToken->function()->token->linenr() == 13);

            functionToken = Token::findsimplematch(tokenizer.tokens(), "foo ( const Fred :: impl & , const Fred :: impl & ) const { }");
            ASSERT(db && functionToken && functionToken->function() &&
                   functionToken->function()->functionScope &&
                   functionToken->function()->tokenDef->linenr() == 8 &&
                   functionToken->function()->token->linenr() == 14);
        }
        {
            GET_SYMBOL_DB("template <typename A> class Fred { struct impl; };\n"
                          "template <typename A> struct Fred<A>::impl {\n"
                          "    impl() { }\n"
                          "    ~impl() { }\n"
                          "    impl(const impl &) { }\n"
                          "    void foo(const impl &, const impl &) const { }\n"
                          "};\n");

            ASSERT(db != nullptr);
            ASSERT(db && db->scopeList.size() == 7);
            ASSERT(db && db->classAndStructScopes.size() == 2);
            ASSERT(db && db->typeList.size() == 2);
            ASSERT(db && db->functionScopes.size() == 4);

            const Token * functionToken = Token::findsimplematch(tokenizer.tokens(), "impl ( ) { }");
            ASSERT(db && functionToken && functionToken->function() &&
                   functionToken->function()->functionScope &&
                   functionToken->function()->tokenDef->linenr() == 3 &&
                   functionToken->function()->token->linenr() == 3);

            functionToken = Token::findsimplematch(tokenizer.tokens(), "~ impl ( ) { }");
            ASSERT(db && functionToken && functionToken->next()->function() &&
                   functionToken->next()->function()->functionScope &&
                   functionToken->next()->function()->tokenDef->linenr() == 4 &&
                   functionToken->next()->function()->token->linenr() == 4);

            functionToken = Token::findsimplematch(tokenizer.tokens(), "impl ( const impl & ) { }");
            ASSERT(db && functionToken && functionToken->function() &&
                   functionToken->function()->functionScope &&
                   functionToken->function()->tokenDef->linenr() == 5 &&
                   functionToken->function()->token->linenr() == 5);

            functionToken = Token::findsimplematch(tokenizer.tokens(), "foo ( const impl & , const impl & ) const { }");
            ASSERT(db && functionToken && functionToken->function() &&
                   functionToken->function()->functionScope &&
                   functionToken->function()->tokenDef->linenr() == 6 &&
                   functionToken->function()->token->linenr() == 6);
        }
        {
            GET_SYMBOL_DB("template <typename A> class Fred { struct impl; };\n"
                          "template <typename A> struct Fred<A>::impl {\n"
                          "    impl();\n"
                          "    ~impl();\n"
                          "    impl(const impl &);\n"
                          "    void foo(const impl &, const impl &) const;\n"
                          "};\n"
                          "template <typename A> Fred<A>::impl::impl() { }\n"
                          "template <typename A> Fred<A>::impl::~impl() { }\n"
                          "template <typename A> Fred<A>::impl::impl(const Fred<A>::impl &) { }\n"
                          "template <typename A> void Fred<A>::impl::foo(const Fred<A>::impl &, const Fred<A>::impl &) const { }\n");

            ASSERT(db != nullptr);
            ASSERT(db && db->scopeList.size() == 7);
            ASSERT(db && db->classAndStructScopes.size() == 2);
            ASSERT(db && db->typeList.size() == 2);
            ASSERT(db && db->functionScopes.size() == 4);

            const Token * functionToken = Token::findsimplematch(tokenizer.tokens(), "impl ( ) { }");
            ASSERT(db && functionToken && functionToken->function() &&
                   functionToken->function()->functionScope &&
                   functionToken->function()->tokenDef->linenr() == 3 &&
                   functionToken->function()->token->linenr() == 8);

            functionToken = Token::findsimplematch(tokenizer.tokens(), "~ impl ( ) { }");
            ASSERT(db && functionToken && functionToken->next()->function() &&
                   functionToken->next()->function()->functionScope &&
                   functionToken->next()->function()->tokenDef->linenr() == 4 &&
                   functionToken->next()->function()->token->linenr() == 9);

            functionToken = Token::findsimplematch(tokenizer.tokens(), "impl ( const Fred < A > :: impl & ) { }");
            ASSERT(db && functionToken && functionToken->function() &&
                   functionToken->function()->functionScope &&
                   functionToken->function()->tokenDef->linenr() == 5 &&
                   functionToken->function()->token->linenr() == 10);

            functionToken = Token::findsimplematch(tokenizer.tokens(), "foo ( const Fred < A > :: impl & , const Fred < A > :: impl & ) const { }");
            ASSERT(db && functionToken && functionToken->function() &&
                   functionToken->function()->functionScope &&
                   functionToken->function()->tokenDef->linenr() == 6 &&
                   functionToken->function()->token->linenr() == 11);
        }
        {
            GET_SYMBOL_DB("namespace NS {\n"
                          "    template <typename A> class Fred { struct impl; };\n"
                          "    template <typename A> struct Fred<A>::impl {\n"
                          "        impl() { }\n"
                          "        ~impl() { }\n"
                          "        impl(const impl &) { }\n"
                          "        void foo(const impl &, const impl &) const { }\n"
                          "    };\n"
                          "}");

            ASSERT(db != nullptr);
            ASSERT(db && db->scopeList.size() == 8);
            ASSERT(db && db->classAndStructScopes.size() == 2);
            ASSERT(db && db->typeList.size() == 2);
            ASSERT(db && db->functionScopes.size() == 4);

            const Token * functionToken = Token::findsimplematch(tokenizer.tokens(), "impl ( ) { }");
            ASSERT(db && functionToken && functionToken->function() &&
                   functionToken->function()->functionScope &&
                   functionToken->function()->tokenDef->linenr() == 4 &&
                   functionToken->function()->token->linenr() == 4);

            functionToken = Token::findsimplematch(tokenizer.tokens(), "~ impl ( ) { }");
            ASSERT(db && functionToken && functionToken->next()->function() &&
                   functionToken->next()->function()->functionScope &&
                   functionToken->next()->function()->tokenDef->linenr() == 5 &&
                   functionToken->next()->function()->token->linenr() == 5);

            functionToken = Token::findsimplematch(tokenizer.tokens(), "impl ( const impl & ) { }");
            ASSERT(db && functionToken && functionToken->function() &&
                   functionToken->function()->functionScope &&
                   functionToken->function()->tokenDef->linenr() == 6 &&
                   functionToken->function()->token->linenr() == 6);

            functionToken = Token::findsimplematch(tokenizer.tokens(), "foo ( const impl & , const impl & ) const { }");
            ASSERT(db && functionToken && functionToken->function() &&
                   functionToken->function()->functionScope &&
                   functionToken->function()->tokenDef->linenr() == 7 &&
                   functionToken->function()->token->linenr() == 7);
        }
        {
            GET_SYMBOL_DB("namespace NS {\n"
                          "    template <typename A> class Fred { struct impl; };\n"
                          "    template <typename A> struct Fred<A>::impl {\n"
                          "        impl();\n"
                          "        ~impl();\n"
                          "        impl(const impl &);\n"
                          "        void foo(const impl &, const impl &) const;\n"
                          "    };\n"
                          "    template <typename A> Fred<A>::impl::impl() { }\n"
                          "    template <typename A> Fred<A>::impl::~impl() { }\n"
                          "    template <typename A> Fred<A>::impl::impl(const Fred<A>::impl &) { }\n"
                          "    template <typename A> void Fred<A>::impl::foo(const Fred<A>::impl &, const Fred<A>::impl &) const { }\n"
                          "}");

            ASSERT(db != nullptr);
            ASSERT(db && db->scopeList.size() == 8);
            ASSERT(db && db->classAndStructScopes.size() == 2);
            ASSERT(db && db->typeList.size() == 2);
            ASSERT(db && db->functionScopes.size() == 4);

            const Token * functionToken = Token::findsimplematch(tokenizer.tokens(), "impl ( ) { }");
            ASSERT(db && functionToken && functionToken->function() &&
                   functionToken->function()->functionScope &&
                   functionToken->function()->tokenDef->linenr() == 4 &&
                   functionToken->function()->token->linenr() == 9);

            functionToken = Token::findsimplematch(tokenizer.tokens(), "~ impl ( ) { }");
            ASSERT(db && functionToken && functionToken->next()->function() &&
                   functionToken->next()->function()->functionScope &&
                   functionToken->next()->function()->tokenDef->linenr() == 5 &&
                   functionToken->next()->function()->token->linenr() == 10);

            functionToken = Token::findsimplematch(tokenizer.tokens(), "impl ( const Fred < A > :: impl & ) { }");
            ASSERT(db && functionToken && functionToken->function() &&
                   functionToken->function()->functionScope &&
                   functionToken->function()->tokenDef->linenr() == 6 &&
                   functionToken->function()->token->linenr() == 11);

            functionToken = Token::findsimplematch(tokenizer.tokens(), "foo ( const Fred < A > :: impl & , const Fred < A > :: impl & ) const { }");
            ASSERT(db && functionToken && functionToken->function() &&
                   functionToken->function()->functionScope &&
                   functionToken->function()->tokenDef->linenr() == 7 &&
                   functionToken->function()->token->linenr() == 12);
        }
        {
            GET_SYMBOL_DB("namespace NS {\n"
                          "    template <typename A> class Fred { struct impl; };\n"
                          "    template <typename A> struct Fred<A>::impl {\n"
                          "        impl();\n"
                          "        ~impl();\n"
                          "        impl(const impl &);\n"
                          "        void foo(const impl &, const impl &) const;\n"
                          "    };\n"
                          "}\n"
                          "template <typename A> NS::Fred<A>::impl::impl() { }\n"
                          "template <typename A> NS::Fred<A>::impl::~impl() { }\n"
                          "template <typename A> NS::Fred<A>::impl::impl(const NS::Fred<A>::impl &) { }\n"
                          "template <typename A> void NS::Fred<A>::impl::foo(const NS::Fred<A>::impl &, const NS::Fred<A>::impl &) const { }\n");

            ASSERT(db != nullptr);
            ASSERT(db && db->scopeList.size() == 8);
            ASSERT(db && db->classAndStructScopes.size() == 2);
            ASSERT(db && db->typeList.size() == 2);
            ASSERT(db && db->functionScopes.size() == 4);

            const Token * functionToken = Token::findsimplematch(tokenizer.tokens(), "impl ( ) { }");
            ASSERT(db && functionToken && functionToken->function() &&
                   functionToken->function()->functionScope &&
                   functionToken->function()->tokenDef->linenr() == 4 &&
                   functionToken->function()->token->linenr() == 10);

            functionToken = Token::findsimplematch(tokenizer.tokens(), "~ impl ( ) { }");
            ASSERT(db && functionToken && functionToken->next()->function() &&
                   functionToken->next()->function()->functionScope &&
                   functionToken->next()->function()->tokenDef->linenr() == 5 &&
                   functionToken->next()->function()->token->linenr() == 11);

            functionToken = Token::findsimplematch(tokenizer.tokens(), "impl ( const NS :: Fred < A > :: impl & ) { }");
            ASSERT(db && functionToken && functionToken->function() &&
                   functionToken->function()->functionScope &&
                   functionToken->function()->tokenDef->linenr() == 6 &&
                   functionToken->function()->token->linenr() == 12);

            functionToken = Token::findsimplematch(tokenizer.tokens(), "foo ( const NS :: Fred < A > :: impl & , const NS :: Fred < A > :: impl & ) const { }");
            ASSERT(db && functionToken && functionToken->function() &&
                   functionToken->function()->functionScope &&
                   functionToken->function()->tokenDef->linenr() == 7 &&
                   functionToken->function()->token->linenr() == 13);
        }
        {
            GET_SYMBOL_DB("namespace NS {\n"
                          "    template <typename A> class Fred { struct impl; };\n"
                          "}\n"
                          "template <typename A> struct NS::Fred<A>::impl {\n"
                          "    impl() { }\n"
                          "    ~impl() { }\n"
                          "    impl(const impl &) { }\n"
                          "    void foo(const impl &, const impl &) const { }\n"
                          "};");

            ASSERT(db != nullptr);
            ASSERT(db && db->scopeList.size() == 8);
            ASSERT(db && db->classAndStructScopes.size() == 2);
            ASSERT(db && db->typeList.size() == 2);
            ASSERT(db && db->functionScopes.size() == 4);

            const Token * functionToken = Token::findsimplematch(tokenizer.tokens(), "impl ( ) { }");
            ASSERT(db && functionToken && functionToken->function() &&
                   functionToken->function()->functionScope &&
                   functionToken->function()->tokenDef->linenr() == 5 &&
                   functionToken->function()->token->linenr() == 5);

            functionToken = Token::findsimplematch(tokenizer.tokens(), "~ impl ( ) { }");
            ASSERT(db && functionToken && functionToken->next()->function() &&
                   functionToken->next()->function()->functionScope &&
                   functionToken->next()->function()->tokenDef->linenr() == 6 &&
                   functionToken->next()->function()->token->linenr() == 6);

            functionToken = Token::findsimplematch(tokenizer.tokens(), "impl ( const impl & ) { }");
            ASSERT(db && functionToken && functionToken->function() &&
                   functionToken->function()->functionScope &&
                   functionToken->function()->tokenDef->linenr() == 7 &&
                   functionToken->function()->token->linenr() == 7);

            functionToken = Token::findsimplematch(tokenizer.tokens(), "foo ( const impl & , const impl & ) const { }");
            ASSERT(db && functionToken && functionToken->function() &&
                   functionToken->function()->functionScope &&
                   functionToken->function()->tokenDef->linenr() == 8 &&
                   functionToken->function()->token->linenr() == 8);
        }
        {
            GET_SYMBOL_DB("namespace NS {\n"
                          "    template <typename A> class Fred { struct impl; };\n"
                          "}\n"
                          "template <typename A> struct NS::Fred<A>::impl {\n"
                          "    impl();\n"
                          "    ~impl();\n"
                          "    impl(const impl &);\n"
                          "    void foo(const impl &, const impl &) const;\n"
                          "};\n"
                          "template <typename A> NS::Fred<A>::impl::impl() { }\n"
                          "template <typename A> NS::Fred<A>::impl::~impl() { }\n"
                          "template <typename A> NS::Fred<A>::impl::impl(const NS::Fred<A>::impl &) { }\n"
                          "template <typename A> void NS::Fred<A>::impl::foo(const NS::Fred<A>::impl &, const NS::Fred<A>::impl &) const { }\n");

            ASSERT(db != nullptr);
            ASSERT(db && db->scopeList.size() == 8);
            ASSERT(db && db->classAndStructScopes.size() == 2);
            ASSERT(db && db->typeList.size() == 2);
            ASSERT(db && db->functionScopes.size() == 4);

            const Token * functionToken = Token::findsimplematch(tokenizer.tokens(), "impl ( ) { }");
            ASSERT(db && functionToken && functionToken->function() &&
                   functionToken->function()->functionScope &&
                   functionToken->function()->tokenDef->linenr() == 5 &&
                   functionToken->function()->token->linenr() == 10);

            functionToken = Token::findsimplematch(tokenizer.tokens(), "~ impl ( ) { }");
            ASSERT(db && functionToken && functionToken->next()->function() &&
                   functionToken->next()->function()->functionScope &&
                   functionToken->next()->function()->tokenDef->linenr() == 6 &&
                   functionToken->next()->function()->token->linenr() == 11);

            functionToken = Token::findsimplematch(tokenizer.tokens(), "impl ( const NS :: Fred < A > :: impl & ) { }");
            ASSERT(db && functionToken && functionToken->function() &&
                   functionToken->function()->functionScope &&
                   functionToken->function()->tokenDef->linenr() == 7 &&
                   functionToken->function()->token->linenr() == 12);

            functionToken = Token::findsimplematch(tokenizer.tokens(), "foo ( const NS :: Fred < A > :: impl & , const NS :: Fred < A > :: impl & ) const { }");
            ASSERT(db && functionToken && functionToken->function() &&
                   functionToken->function()->functionScope &&
                   functionToken->function()->tokenDef->linenr() == 8 &&
                   functionToken->function()->token->linenr() == 13);
        }
        {
            GET_SYMBOL_DB("namespace NS {\n"
                          "    template <typename A> class Fred { struct impl; };\n"
                          "}\n"
                          "template <typename A> struct NS::Fred<A>::impl {\n"
                          "    impl();\n"
                          "    ~impl();\n"
                          "    impl(const impl &);\n"
                          "    void foo(const impl &, const impl &) const;\n"
                          "};\n"
                          "namespace NS {\n"
                          "    template <typename A> Fred<A>::impl::impl() { }\n"
                          "    template <typename A> Fred<A>::impl::~impl() { }\n"
                          "    template <typename A> Fred<A>::impl::impl(const Fred<A>::impl &) { }\n"
                          "    template <typename A> void Fred<A>::impl::foo(const Fred<A>::impl &, const Fred<A>::impl &) const { }\n"
                          "}");

            ASSERT(db != nullptr);
            ASSERT(db && db->scopeList.size() == 8);
            ASSERT(db && db->classAndStructScopes.size() == 2);
            ASSERT(db && db->typeList.size() == 2);
            ASSERT(db && db->functionScopes.size() == 4);

            const Token * functionToken = Token::findsimplematch(tokenizer.tokens(), "impl ( ) { }");
            ASSERT(db && functionToken && functionToken->function() &&
                   functionToken->function()->functionScope &&
                   functionToken->function()->tokenDef->linenr() == 5 &&
                   functionToken->function()->token->linenr() == 11);

            functionToken = Token::findsimplematch(tokenizer.tokens(), "~ impl ( ) { }");
            ASSERT(db && functionToken && functionToken->next()->function() &&
                   functionToken->next()->function()->functionScope &&
                   functionToken->next()->function()->tokenDef->linenr() == 6 &&
                   functionToken->next()->function()->token->linenr() == 12);

            functionToken = Token::findsimplematch(tokenizer.tokens(), "impl ( const Fred < A > :: impl & ) { }");
            ASSERT(db && functionToken && functionToken->function() &&
                   functionToken->function()->functionScope &&
                   functionToken->function()->tokenDef->linenr() == 7 &&
                   functionToken->function()->token->linenr() == 13);

            functionToken = Token::findsimplematch(tokenizer.tokens(), "foo ( const Fred < A > :: impl & , const Fred < A > :: impl & ) const { }");
            ASSERT(db && functionToken && functionToken->function() &&
                   functionToken->function()->functionScope &&
                   functionToken->function()->tokenDef->linenr() == 8 &&
                   functionToken->function()->token->linenr() == 14);
        }
        {
            GET_SYMBOL_DB("namespace NS {\n"
                          "    template <typename A> class Fred { struct impl; };\n"
                          "}\n"
                          "template <typename A> struct NS::Fred::impl {\n"
                          "    impl();\n"
                          "    ~impl();\n"
                          "    impl(const impl &);\n"
                          "    void foo(const impl &, const impl &) const;\n"
                          "};\n"
                          "using namespace NS;\n"
                          "template <typename A> Fred<A>::impl::impl() { }\n"
                          "template <typename A> Fred<A>::impl::~impl() { }\n"
                          "template <typename A> Fred<A>::impl::impl(const Fred<A>::impl &) { }\n"
                          "template <typename A> void Fred<A>::impl::foo(const Fred<A>::impl &, const Fred<A>::impl &) const { }\n");

            ASSERT(db != nullptr);
            ASSERT(db && db->scopeList.size() == 8);
            ASSERT(db && db->classAndStructScopes.size() == 2);
            ASSERT(db && db->typeList.size() == 2);
            ASSERT(db && db->functionScopes.size() == 4);

            const Token * functionToken = Token::findsimplematch(tokenizer.tokens(), "impl ( ) { }");
            ASSERT(db && functionToken && functionToken->function() &&
                   functionToken->function()->functionScope &&
                   functionToken->function()->tokenDef->linenr() == 5 &&
                   functionToken->function()->token->linenr() == 11);

            functionToken = Token::findsimplematch(tokenizer.tokens(), "~ impl ( ) { }");
            ASSERT(db && functionToken && functionToken->next()->function() &&
                   functionToken->next()->function()->functionScope &&
                   functionToken->next()->function()->tokenDef->linenr() == 6 &&
                   functionToken->next()->function()->token->linenr() == 12);

            functionToken = Token::findsimplematch(tokenizer.tokens(), "impl ( const Fred < A > :: impl & ) { }");
            ASSERT(db && functionToken && functionToken->function() &&
                   functionToken->function()->functionScope &&
                   functionToken->function()->tokenDef->linenr() == 7 &&
                   functionToken->function()->token->linenr() == 13);

            functionToken = Token::findsimplematch(tokenizer.tokens(), "foo ( const Fred < A > :: impl & , const Fred < A > :: impl & ) const { }");
            ASSERT(db && functionToken && functionToken->function() &&
                   functionToken->function()->functionScope &&
                   functionToken->function()->tokenDef->linenr() == 8 &&
                   functionToken->function()->token->linenr() == 14);
        }
    }

    void symboldatabase65() {
        // don't crash on missing links from instantiation of template with typedef
        check("int ( * X0 ) ( long ) < int ( ) ( long ) > :: f0 ( int * ) { return 0 ; }");
        ASSERT_EQUALS("[test.cpp:1]: (debug) SymbolDatabase::findFunction found '>' without link.\n", errout.str());
    }

    void symboldatabase66() { // #8540
        GET_SYMBOL_DB("enum class ENUM1;\n"
                      "enum class ENUM2 { MEMBER2 };\n"
                      "enum class ENUM3 : int { MEMBER1, };");
        ASSERT(db != nullptr);
        ASSERT(db && db->scopeList.size() == 3);
        ASSERT(db && db->typeList.size() == 3);
    }

    void symboldatabase67() { // #8538
        GET_SYMBOL_DB("std::string get_endpoint_url() const noexcept override;");
        const Function *f = db ? &db->scopeList.front().functionList.front() : nullptr;
        ASSERT(f != nullptr);
        ASSERT(f && f->hasOverrideSpecifier());
        ASSERT(f && f->isConst());
        ASSERT(f && f->isNoExcept());
    }

    void symboldatabase68() { // #8560
        GET_SYMBOL_DB("struct Bar {\n"
                      "    virtual std::string get_endpoint_url() const noexcept;\n"
                      "};\n"
                      "struct Foo : Bar {\n"
                      "    virtual std::string get_endpoint_url() const noexcept override final;\n"
                      "};");
        const Token *f = db ? Token::findsimplematch(tokenizer.tokens(), "get_endpoint_url ( ) const noexcept ;") : nullptr;
        ASSERT(f != nullptr);
        ASSERT(f && f->function() && f->function()->token->linenr() == 2);
        ASSERT(f && f->function() && f->function()->isVirtual());
        ASSERT(f && f->function() && !f->function()->hasOverrideSpecifier());
        ASSERT(f && f->function() && !f->function()->hasFinalSpecifier());
        ASSERT(f && f->function() && f->function()->isConst());
        ASSERT(f && f->function() && f->function()->isNoExcept());
        f = db ? Token::findsimplematch(tokenizer.tokens(), "get_endpoint_url ( ) const noexcept override final ;") : nullptr;
        ASSERT(f != nullptr);
        ASSERT(f && f->function() && f->function()->token->linenr() == 5);
        ASSERT(f && f->function() && f->function()->isVirtual());
        ASSERT(f && f->function() && f->function()->hasOverrideSpecifier());
        ASSERT(f && f->function() && f->function()->hasFinalSpecifier());
        ASSERT(f && f->function() && f->function()->isConst());
        ASSERT(f && f->function() && f->function()->isNoExcept());
    }

    void symboldatabase69() {
        GET_SYMBOL_DB("struct Fred {\n"
                      "    int x, y;\n"
                      "    void foo() const volatile { }\n"
                      "    void foo() volatile { }\n"
                      "    void foo() const { }\n"
                      "    void foo() { }\n"
                      "};");
        const Token *f = db ? Token::findsimplematch(tokenizer.tokens(), "foo ( ) const volatile {") : nullptr;
        ASSERT(f != nullptr);
        ASSERT(f && f->function() && f->function()->token->linenr() == 3);
        ASSERT(f && f->function() && f->function()->isConst());
        ASSERT(f && f->function() && f->function()->isVolatile());
        f = db ? Token::findsimplematch(tokenizer.tokens(), "foo ( ) volatile {") : nullptr;
        ASSERT(f != nullptr);
        ASSERT(f && f->function() && f->function()->token->linenr() == 4);
        ASSERT(f && f->function() && !f->function()->isConst());
        ASSERT(f && f->function() && f->function()->isVolatile());
        f = db ? Token::findsimplematch(tokenizer.tokens(), "foo ( ) const {") : nullptr;
        ASSERT(f != nullptr);
        ASSERT(f && f->function() && f->function()->token->linenr() == 5);
        ASSERT(f && f->function() && f->function()->isConst());
        ASSERT(f && f->function() && !f->function()->isVolatile());
        f = db ? Token::findsimplematch(tokenizer.tokens(), "foo ( ) {") : nullptr;
        ASSERT(f != nullptr);
        ASSERT(f && f->function() && f->function()->token->linenr() == 6);
        ASSERT(f && f->function() && !f->function()->isVolatile());
        ASSERT(f && f->function() && !f->function()->isConst());
    }

    void symboldatabase70() {
        {
            GET_SYMBOL_DB("class Map<String,Entry>::Entry* e;");
            ASSERT(db != nullptr);
            ASSERT(db && db->scopeList.size() == 1);
            ASSERT(db && db->variableList().size() == 2);
        }
        {
            GET_SYMBOL_DB("template class boost::token_iterator_generator<boost::offset_separator>::type; void foo() { }");
            ASSERT(db != nullptr);
            ASSERT(db && db->scopeList.size() == 2);
        }
        {
            GET_SYMBOL_DB("void foo() {\n"
                          "    return class Arm_relocate_functions<big_endian>::thumb32_branch_offset(upper_insn, lower_insn);\n"
                          "}");
            ASSERT(db != nullptr);
            ASSERT(db && db->scopeList.size() == 2);
        }
    }

    void symboldatabase71() {
        GET_SYMBOL_DB("class A { };\n"
                      "class B final : public A { };");
        ASSERT(db && db->scopeList.size() == 3);
        ASSERT(db && db->typeList.size() == 2);
    }

    void symboldatabase72() { // #8600
        GET_SYMBOL_DB("struct A { struct B; };\n"
                      "struct A::B {\n"
                      "    B() = default;\n"
                      "    B(const B&) {}\n"
                      "};");

        ASSERT(db && db->scopeList.size() == 4);
        ASSERT(db && db->typeList.size() == 2);
        const Token * f = db ? Token::findsimplematch(tokenizer.tokens(), "B ( const B & ) { }") : nullptr;
        ASSERT(f != nullptr);
        ASSERT(f && f->function() && f->function()->token->linenr() == 4);
        ASSERT(f && f->function() && f->function()->type == Function::eCopyConstructor);
    }

    void symboldatabase73() { // #8603
        GET_SYMBOL_DB("namespace swizzle {\n"
                      "  template <comp> void swizzle(tvec2<f16>) {}\n"
                      "  template <comp x, comp y> void swizzle(tvec2<f16> v) {}\n"
                      "}");

        ASSERT_EQUALS(4, db->scopeList.size());
        ASSERT_EQUALS(2, db->functionScopes.size());

        const Scope *f1 = db->functionScopes[0];
        ASSERT_EQUALS(2, f1->bodyStart->linenr());
        ASSERT_EQUALS(2, f1->bodyEnd->linenr());
        ASSERT_EQUALS(2, f1->function->token->linenr());

        const Scope *f2 = db->functionScopes[1];
        ASSERT_EQUALS(3, f2->bodyStart->linenr());
        ASSERT_EQUALS(3, f2->bodyEnd->linenr());
        ASSERT_EQUALS(3, f2->function->token->linenr());
    }

    void createSymbolDatabaseFindAllScopes1() {
        GET_SYMBOL_DB("void f() { union {int x; char *p;} a={0}; }");
        ASSERT(db->scopeList.size() == 3);
        ASSERT_EQUALS(Scope::eUnion, db->scopeList.back().type);
    }

    void enum1() {
        GET_SYMBOL_DB("enum BOOL { FALSE, TRUE }; enum BOOL b;");

        /* there is a enum scope with the name BOOL */
        ASSERT(db && db->scopeList.back().type == Scope::eEnum && db->scopeList.back().className == "BOOL");

        /* b is a enum variable, type is BOOL */
        ASSERT(db && db->getVariableFromVarId(1)->isEnumType());
    }

    void enum2() {
        GET_SYMBOL_DB("enum BOOL { FALSE, TRUE } b;");

        /* there is a enum scope with the name BOOL */
        ASSERT(db && db->scopeList.back().type == Scope::eEnum && db->scopeList.back().className == "BOOL");

        /* b is a enum variable, type is BOOL */
        ASSERT(db && db->getVariableFromVarId(1)->isEnumType());
    }

    void enum3() {
        GET_SYMBOL_DB("enum ABC { A=11,B,C=A+B };");
        ASSERT(db && db->scopeList.back().type == Scope::eEnum);
        if (db) {
            /* There is an enum A with value 11 */
            const Enumerator *A = db->scopeList.back().findEnumerator("A");
            ASSERT(A && A->value==11 && A->value_known);

            /* There is an enum B with value 12 */
            const Enumerator *B = db->scopeList.back().findEnumerator("B");
            ASSERT(B && B->value==12 && B->value_known);

            /* There is an enum C with value 23 */
            const Enumerator *C = db->scopeList.back().findEnumerator("C");
            ASSERT(C && C->value==23 && C->value_known);
        }
    }

    void enum4() { // #7493
        GET_SYMBOL_DB("enum Offsets { O1, O2, O3=5, O4 };\n"
                      "enum MyEnums { E1=O1+1, E2, E3=O3+1 };");
        ASSERT(db != nullptr);
        if (!db)
            return;
        ASSERT_EQUALS(3U, db->scopeList.size());

        // Assert that all enum values are known
        std::list<Scope>::const_iterator scope = db->scopeList.begin();

        // Offsets
        ++scope;
        ASSERT_EQUALS((unsigned int)Scope::eEnum, (unsigned int)scope->type);
        ASSERT_EQUALS(4U, scope->enumeratorList.size());

        ASSERT(scope->enumeratorList[0].name->enumerator() == &scope->enumeratorList[0]);
        ASSERT_EQUALS((unsigned int)Token::eEnumerator, (unsigned int)scope->enumeratorList[0].name->tokType());
        ASSERT(scope->enumeratorList[0].scope == &*scope);
        ASSERT_EQUALS("O1", scope->enumeratorList[0].name->str());
        ASSERT(scope->enumeratorList[0].start == nullptr);
        ASSERT(scope->enumeratorList[0].end == nullptr);
        ASSERT_EQUALS(true, scope->enumeratorList[0].value_known);
        ASSERT_EQUALS(0, scope->enumeratorList[0].value);

        ASSERT(scope->enumeratorList[1].name->enumerator() == &scope->enumeratorList[1]);
        ASSERT_EQUALS((unsigned int)Token::eEnumerator, (unsigned int)scope->enumeratorList[1].name->tokType());
        ASSERT(scope->enumeratorList[1].scope == &*scope);
        ASSERT_EQUALS("O2", scope->enumeratorList[1].name->str());
        ASSERT(scope->enumeratorList[1].start == nullptr);
        ASSERT(scope->enumeratorList[1].end == nullptr);
        ASSERT_EQUALS(true, scope->enumeratorList[1].value_known);
        ASSERT_EQUALS(1, scope->enumeratorList[1].value);

        ASSERT(scope->enumeratorList[2].name->enumerator() == &scope->enumeratorList[2]);
        ASSERT_EQUALS((unsigned int)Token::eEnumerator, (unsigned int)scope->enumeratorList[2].name->tokType());
        ASSERT(scope->enumeratorList[2].scope == &*scope);
        ASSERT_EQUALS("O3", scope->enumeratorList[2].name->str());
        ASSERT(scope->enumeratorList[2].start != nullptr);
        ASSERT(scope->enumeratorList[2].end != nullptr);
        ASSERT_EQUALS(true, scope->enumeratorList[2].value_known);
        ASSERT_EQUALS(5, scope->enumeratorList[2].value);

        ASSERT(scope->enumeratorList[3].name->enumerator() == &scope->enumeratorList[3]);
        ASSERT_EQUALS((unsigned int)Token::eEnumerator, (unsigned int)scope->enumeratorList[3].name->tokType());
        ASSERT(scope->enumeratorList[3].scope == &*scope);
        ASSERT_EQUALS("O4", scope->enumeratorList[3].name->str());
        ASSERT(scope->enumeratorList[3].start == nullptr);
        ASSERT(scope->enumeratorList[3].end == nullptr);
        ASSERT_EQUALS(true, scope->enumeratorList[3].value_known);
        ASSERT_EQUALS(6, scope->enumeratorList[3].value);

        // MyEnums
        ++scope;
        ASSERT_EQUALS((unsigned int)Scope::eEnum, (unsigned int)scope->type);
        ASSERT_EQUALS(3U, scope->enumeratorList.size());

        ASSERT(scope->enumeratorList[0].name->enumerator() == &scope->enumeratorList[0]);
        ASSERT_EQUALS((unsigned int)Token::eEnumerator, (unsigned int)scope->enumeratorList[0].name->tokType());
        ASSERT(scope->enumeratorList[0].scope == &*scope);
        ASSERT_EQUALS("E1", scope->enumeratorList[0].name->str());
        ASSERT(scope->enumeratorList[0].start != nullptr);
        ASSERT(scope->enumeratorList[0].end != nullptr);
        ASSERT_EQUALS(true, scope->enumeratorList[0].value_known);
        ASSERT_EQUALS(1, scope->enumeratorList[0].value);

        ASSERT(scope->enumeratorList[1].name->enumerator() == &scope->enumeratorList[1]);
        ASSERT_EQUALS((unsigned int)Token::eEnumerator, (unsigned int)scope->enumeratorList[1].name->tokType());
        ASSERT(scope->enumeratorList[1].scope == &*scope);
        ASSERT_EQUALS("E2", scope->enumeratorList[1].name->str());
        ASSERT(scope->enumeratorList[1].start == nullptr);
        ASSERT(scope->enumeratorList[1].end == nullptr);
        ASSERT_EQUALS(true, scope->enumeratorList[1].value_known);
        ASSERT_EQUALS(2, scope->enumeratorList[1].value);

        ASSERT(scope->enumeratorList[2].name->enumerator() == &scope->enumeratorList[2]);
        ASSERT_EQUALS((unsigned int)Token::eEnumerator, (unsigned int)scope->enumeratorList[2].name->tokType());
        ASSERT(scope->enumeratorList[2].scope == &*scope);
        ASSERT_EQUALS("E3", scope->enumeratorList[2].name->str());
        ASSERT(scope->enumeratorList[2].start != nullptr);
        ASSERT(scope->enumeratorList[2].end != nullptr);
        ASSERT_EQUALS(true, scope->enumeratorList[2].value_known);
        ASSERT_EQUALS(6, scope->enumeratorList[2].value);
    }

    void enum5() {
        GET_SYMBOL_DB("enum { A = 10, B = 2 };\n"
                      "int a[10 + 2];\n"
                      "int b[A];\n"
                      "int c[A + 2];\n"
                      "int d[10 + B];\n"
                      "int e[A + B];\n");
        ASSERT(db != nullptr);
        if (!db)
            return;
        ASSERT_EQUALS(2U, db->scopeList.size());

        // Assert that all enum values are known
        std::list<Scope>::const_iterator scope = db->scopeList.begin();

        ++scope;
        ASSERT_EQUALS((unsigned int)Scope::eEnum, (unsigned int)scope->type);
        ASSERT_EQUALS(2U, scope->enumeratorList.size());
        ASSERT_EQUALS(true, scope->enumeratorList[0].value_known);
        ASSERT_EQUALS(10, scope->enumeratorList[0].value);
        ASSERT_EQUALS(true, scope->enumeratorList[1].value_known);
        ASSERT_EQUALS(2, scope->enumeratorList[1].value);

        ASSERT(db->variableList().size() == 6); // the first one is not used
        const Variable * v = db->getVariableFromVarId(1);
        ASSERT(v != nullptr);
        if (!v)
            return;
        ASSERT(v->isArray());
        ASSERT_EQUALS(1U, v->dimensions().size());
        ASSERT_EQUALS(12U, v->dimension(0));
        v = db->getVariableFromVarId(2);
        ASSERT(v != nullptr);
        if (!v)
            return;
        ASSERT(v->isArray());
        ASSERT_EQUALS(1U, v->dimensions().size());
        ASSERT_EQUALS(10U, v->dimension(0));
        v = db->getVariableFromVarId(3);
        ASSERT(v != nullptr);
        if (!v)
            return;
        ASSERT(v->isArray());
        ASSERT_EQUALS(1U, v->dimensions().size());
        ASSERT_EQUALS(12U, v->dimension(0));
        v = db->getVariableFromVarId(4);
        ASSERT(v != nullptr);
        if (!v)
            return;
        ASSERT(v->isArray());
        ASSERT_EQUALS(1U, v->dimensions().size());
        ASSERT_EQUALS(12U, v->dimension(0));
        v = db->getVariableFromVarId(5);
        ASSERT(v != nullptr);
        if (!v)
            return;
        ASSERT(v->isArray());
        ASSERT_EQUALS(1U, v->dimensions().size());
        ASSERT_EQUALS(12U, v->dimension(0));
    }

    void enum6() {
        GET_SYMBOL_DB("struct Fred {\n"
                      "    enum Enum { E0, E1 };\n"
                      "};\n"
                      "struct Barney : public Fred {\n"
                      "    Enum func(Enum e) { return e; }\n"
                      "};");
        ASSERT(db != nullptr);
        if (!db)
            return;
        const Token * const functionToken = Token::findsimplematch(tokenizer.tokens(), "func");
        ASSERT(functionToken != nullptr);
        if (!functionToken)
            return;
        const Function *function = functionToken->function();
        ASSERT(function != nullptr);
        if (!function)
            return;
        ASSERT(function->token->str() == "func");
        ASSERT(function->retDef && function->retDef->str() == "Enum");
        ASSERT(function->retType && function->retType->name() == "Enum");
    }

#define TEST(S) \
        v = db->getVariableFromVarId(id++); \
        ASSERT(v != nullptr); \
        if (!v) \
            return; \
        ASSERT(v->isArray()); \
        ASSERT_EQUALS(1U, v->dimensions().size()); \
        ASSERT_EQUALS(S, v->dimension(0))

    void enum7() {
        GET_SYMBOL_DB("enum E { X };\n"
                      "enum EC : char { C };\n"
                      "enum ES : short { S };\n"
                      "enum EI : int { I };\n"
                      "enum EL : long { L };\n"
                      "enum ELL : long long { LL };\n"
                      "char array1[sizeof(E)];\n"
                      "char array2[sizeof(X)];\n"
                      "char array3[sizeof(EC)];\n"
                      "char array4[sizeof(C)];\n"
                      "char array5[sizeof(ES)];\n"
                      "char array6[sizeof(S)];\n"
                      "char array7[sizeof(EI)];\n"
                      "char array8[sizeof(I)];\n"
                      "char array9[sizeof(EL)];\n"
                      "char array10[sizeof(L)];\n"
                      "char array11[sizeof(ELL)];\n"
                      "char array12[sizeof(LL)];\n");
        ASSERT(db != nullptr);
        if (!db)
            return;
        ASSERT(db->variableList().size() == 13); // the first one is not used
        const Variable * v;
        unsigned int id = 1;
        TEST(settings1.sizeof_int);
        TEST(settings1.sizeof_int);
        TEST(1);
        TEST(1);
        TEST(settings1.sizeof_short);
        TEST(settings1.sizeof_short);
        TEST(settings1.sizeof_int);
        TEST(settings1.sizeof_int);
        TEST(settings1.sizeof_long);
        TEST(settings1.sizeof_long);
        TEST(settings1.sizeof_long_long);
        TEST(settings1.sizeof_long_long);
    }

    void sizeOfType() {
        // #7615 - crash in Symboldatabase::sizeOfType()
        GET_SYMBOL_DB("enum e;\n"
                      "void foo() {\n"
                      "    e abc[] = {A,B,C};\n"
                      "    int i = abc[ARRAY_SIZE(cats)];\n"
                      "}");
        const Token *e = Token::findsimplematch(tokenizer.tokens(), "e abc");
        db->sizeOfType(e);  // <- don't crash
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
            ASSERT(db && db->findScopeByName("Bar") && !db->findScopeByName("Bar")->functionList.empty() && !db->findScopeByName("Bar")->functionList.front().isImplicitlyVirtual(false));
            if (db && db->findScopeByName("Bar"))
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

    void isPure() {
        GET_SYMBOL_DB("class C {\n"
                      "    void f() = 0;\n"
                      "    C(B b) = 0;\n"
                      "    C(C& c) = default;"
                      "    void g();\n"
                      "};");
        ASSERT(db && db->scopeList.back().functionList.size() == 4);
        if (db && db->scopeList.back().functionList.size() == 4) {
            std::list<Function>::const_iterator it = db->scopeList.back().functionList.begin();
            ASSERT((it++)->isPure());
            ASSERT((it++)->isPure());
            ASSERT(!(it++)->isPure());
            ASSERT(!(it++)->isPure());
        }
    }

    void isFunction1() { // #5602 - UNKNOWN_MACRO(a,b) { .. }
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

    void isFunction2() {
        GET_SYMBOL_DB("void set_cur_cpu_spec()\n"
                      "{\n"
                      "    t = PTRRELOC(t);\n"
                      "}\n"
                      "\n"
                      "cpu_spec * __init setup_cpu_spec()\n"
                      "{\n"
                      "    t = PTRRELOC(t);\n"
                      "    *PTRRELOC(&x) = &y;\n"
                      "}");
        ASSERT(db != nullptr);
        ASSERT(db && !db->isFunction(Token::findsimplematch(tokenizer.tokens(), "PTRRELOC ( &"), &db->scopeList.back(), nullptr, nullptr, nullptr));
        ASSERT(db->findScopeByName("set_cur_cpu_spec") != nullptr);
        ASSERT(db->findScopeByName("setup_cpu_spec") != nullptr);
        ASSERT(db->findScopeByName("PTRRELOC") == nullptr);
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
                const unsigned int linenrs[2] = { 2, 1 };
                unsigned int index = 0;
                for (const Token * tok = bar->bodyStart->next(); tok != bar->bodyEnd; tok = tok->next()) {
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

    void findFunction7() {
        GET_SYMBOL_DB("class ResultEnsemble {\n"
                      "public:\n"
                      "    std::vector<int> &nodeResults() const;\n"
                      "    std::vector<int> &nodeResults();\n"
                      "};\n"
                      "class Simulator {\n"
                      "    int generatePinchResultEnsemble(const ResultEnsemble &power, const ResultEnsemble &ground) {\n"
                      "        power.nodeResults().size();\n"
                      "        assert(power.nodeResults().size()==ground.nodeResults().size());\n"
                      "    }\n"
                      "};")
        const Token *callfunc = Token::findsimplematch(tokenizer.tokens(), "power . nodeResults ( ) . size ( ) ;");
        ASSERT_EQUALS("", errout.str());
        ASSERT_EQUALS(true, db != nullptr); // not null
        ASSERT_EQUALS(true, callfunc != nullptr); // not null
        ASSERT_EQUALS(true, callfunc && callfunc->tokAt(2)->function() && callfunc->tokAt(2)->function()->tokenDef->linenr() == 3);
    }

    void findFunction8() {
        GET_SYMBOL_DB("struct S {\n"
                      "    void f()   { }\n"
                      "    void f() & { }\n"
                      "    void f() &&{ }\n"
                      "    void f() const   { }\n"
                      "    void f() const & { }\n"
                      "    void f() const &&{ }\n"
                      "    void g()   ;\n"
                      "    void g() & ;\n"
                      "    void g() &&;\n"
                      "    void g() const   ;\n"
                      "    void g() const & ;\n"
                      "    void g() const &&;\n"
                      "};\n"
                      "void S::g()   { }\n"
                      "void S::g() & { }\n"
                      "void S::g() &&{ }\n"
                      "void S::g() const   { }\n"
                      "void S::g() const & { }\n"
                      "void S::g() const &&{ }\n");
        ASSERT_EQUALS("", errout.str());

        const Token *f = Token::findsimplematch(tokenizer.tokens(), "f ( ) {");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 2);

        f = Token::findsimplematch(tokenizer.tokens(), "f ( ) & {");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 3);

        f = Token::findsimplematch(tokenizer.tokens(), "f ( ) && {");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 4);

        f = Token::findsimplematch(tokenizer.tokens(), "f ( ) const {");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 5);

        f = Token::findsimplematch(tokenizer.tokens(), "f ( ) const & {");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 6);

        f = Token::findsimplematch(tokenizer.tokens(), "f ( ) const && {");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 7);

        f = Token::findsimplematch(tokenizer.tokens(), "g ( ) {");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 8 && f->function()->token->linenr() == 15);

        f = Token::findsimplematch(tokenizer.tokens(), "g ( ) & {");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 9 && f->function()->token->linenr() == 16);

        f = Token::findsimplematch(tokenizer.tokens(), "g ( ) && {");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 10 && f->function()->token->linenr() == 17);

        f = Token::findsimplematch(tokenizer.tokens(), "g ( ) const {");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 11 && f->function()->token->linenr() == 18);

        f = Token::findsimplematch(tokenizer.tokens(), "g ( ) const & {");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 12 && f->function()->token->linenr() == 19);

        f = Token::findsimplematch(tokenizer.tokens(), "g ( ) const && {");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 13 && f->function()->token->linenr() == 20);

        f = Token::findsimplematch(tokenizer.tokens(), "S :: g ( ) {");
        ASSERT_EQUALS(true, db && f && f->tokAt(2)->function() && f->tokAt(2)->function()->tokenDef->linenr() == 8 && f->tokAt(2)->function()->token->linenr() == 15);

        f = Token::findsimplematch(tokenizer.tokens(), "S :: g ( ) & {");
        ASSERT_EQUALS(true, db && f && f->tokAt(2)->function() && f->tokAt(2)->function()->tokenDef->linenr() == 9 && f->tokAt(2)->function()->token->linenr() == 16);

        f = Token::findsimplematch(tokenizer.tokens(), "S :: g ( ) && {");
        ASSERT_EQUALS(true, db && f && f->tokAt(2)->function() && f->tokAt(2)->function()->tokenDef->linenr() == 10 && f->tokAt(2)->function()->token->linenr() == 17);

        f = Token::findsimplematch(tokenizer.tokens(), "S :: g ( ) const {");
        ASSERT_EQUALS(true, db && f && f->tokAt(2)->function() && f->tokAt(2)->function()->tokenDef->linenr() == 11 && f->tokAt(2)->function()->token->linenr() == 18);

        f = Token::findsimplematch(tokenizer.tokens(), "S :: g ( ) const & {");
        ASSERT_EQUALS(true, db && f && f->tokAt(2)->function() && f->tokAt(2)->function()->tokenDef->linenr() == 12 && f->tokAt(2)->function()->token->linenr() == 19);

        f = Token::findsimplematch(tokenizer.tokens(), "S :: g ( ) const && {");
        ASSERT_EQUALS(true, db && f && f->tokAt(2)->function() && f->tokAt(2)->function()->tokenDef->linenr() == 13 && f->tokAt(2)->function()->token->linenr() == 20);
    }

    void findFunction9() {
        GET_SYMBOL_DB("struct Fred {\n"
                      "    void foo(const int * p);\n"
                      "};\n"
                      "void Fred::foo(const int * const p) { }");
        ASSERT_EQUALS("", errout.str());

        const Token *f = Token::findsimplematch(tokenizer.tokens(), "foo ( const int * const p ) {");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 2);
    }

    void findFunction10() { // #7673
        GET_SYMBOL_DB("struct Fred {\n"
                      "    void foo(const int * p);\n"
                      "};\n"
                      "void Fred::foo(const int p []) { }");
        ASSERT_EQUALS("", errout.str());

        const Token *f = Token::findsimplematch(tokenizer.tokens(), "foo ( const int p [ ] ) {");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 2);
    }

    void findFunction11() {
        GET_SYMBOL_DB("class Fred : public QObject {\n"
                      "    Q_OBJECT\n"
                      "private slots:\n"
                      "    void foo();\n"
                      "};\n"
                      "void Fred::foo() { }");
        ASSERT_EQUALS("", errout.str());

        const Token *f = Token::findsimplematch(tokenizer.tokens(), "foo ( ) {");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 4);
    }

    void findFunction12() {
        GET_SYMBOL_DB("void foo(std::string a) { }\n"
                      "void foo(long long a) { }\n"
                      "void func(char* cp) {\n"
                      "    foo(0);\n"
                      "    foo(0L);\n"
                      "    foo(0.f);\n"
                      "    foo(bar());\n"
                      "    foo(cp);\n"
                      "    foo(\"\");\n"
                      "}");

        ASSERT_EQUALS("", errout.str());

        const Token *f = Token::findsimplematch(tokenizer.tokens(), "foo ( 0 ) ;");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 2);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( 0L ) ;");
        ASSERT_EQUALS(true, f && f->function() && f->function()->tokenDef->linenr() == 2);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( 0.f ) ;");
        ASSERT_EQUALS(true, f && f->function() && f->function()->tokenDef->linenr() == 2);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( bar ( ) ) ;");
        ASSERT_EQUALS(true, f && f->function() == nullptr);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( cp ) ;");
        ASSERT_EQUALS(true, f && f->function() && f->function()->tokenDef->linenr() == 1);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( \"\" ) ;");
        ASSERT_EQUALS(true, f && f->function() && f->function()->tokenDef->linenr() == 1);
    }

    void findFunction13() {
        GET_SYMBOL_DB("void foo(std::string a) { }\n"
                      "void foo(double a) { }\n"
                      "void foo(long long a) { }\n"
                      "void foo(int* a) { }\n"
                      "void foo(void* a) { }\n"
                      "void func(int i, const float f, int* ip, float* fp, char* cp) {\n"
                      "    foo(0);\n"
                      "    foo(0L);\n"
                      "    foo(0.f);\n"
                      "    foo(false);\n"
                      "    foo(bar());\n"
                      "    foo(i);\n"
                      "    foo(f);\n"
                      "    foo(&i);\n"
                      "    foo(&f);\n"
                      "    foo(ip);\n"
                      "    foo(fp);\n"
                      "    foo(cp);\n"
                      "    foo(\"\");\n"
                      "}");

        ASSERT_EQUALS("", errout.str());

        const Token *f = Token::findsimplematch(tokenizer.tokens(), "foo ( 0 ) ;");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 3);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( 0L ) ;");
        ASSERT_EQUALS(true, f && f->function() && f->function()->tokenDef->linenr() == 3);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( 0.f ) ;");
        ASSERT_EQUALS(true, f && f->function() && f->function()->tokenDef->linenr() == 2);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( false ) ;");
        ASSERT_EQUALS(true, f && f->function() && f->function()->tokenDef->linenr() == 3);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( bar ( ) ) ;");
        ASSERT_EQUALS(true, f && f->function() == nullptr);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( i ) ;");
        ASSERT_EQUALS(true, f && f->function() && f->function()->tokenDef->linenr() == 3);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( f ) ;");
        ASSERT_EQUALS(true, f && f->function() && f->function()->tokenDef->linenr() == 2);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( & i ) ;");
        ASSERT_EQUALS(true, f && f->function() && f->function()->tokenDef->linenr() == 4);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( & f ) ;");
        ASSERT_EQUALS(true, f && f->function() && f->function()->tokenDef->linenr() == 5);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( ip ) ;");
        ASSERT_EQUALS(true, f && f->function() && f->function()->tokenDef->linenr() == 4);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( fp ) ;");
        ASSERT_EQUALS(true, f && f->function() && f->function()->tokenDef->linenr() == 5);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( cp ) ;");
        ASSERT_EQUALS(true, f && f->function() && f->function()->tokenDef->linenr() == 5);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( \"\" ) ;");
        ASSERT_EQUALS(true, f && f->function() && f->function()->tokenDef->linenr() == 5);
    }

    void findFunction14() {
        GET_SYMBOL_DB("void foo(int* a) { }\n"
                      "void foo(const int* a) { }\n"
                      "void foo(void* a) { }\n"
                      "void foo(const float a) { }\n"
                      "void foo(bool a) { }\n"
                      "void foo2(Foo* a) { }\n"
                      "void foo2(Foo a) { }\n"
                      "void func(int* ip, const int* cip, const char* ccp, char* cp, float f, bool b) {\n"
                      "    foo(ip);\n"
                      "    foo(cip);\n"
                      "    foo(cp);\n"
                      "    foo(ccp);\n"
                      "    foo(f);\n"
                      "    foo(b);\n"
                      "    foo2(0);\n"
                      "    foo2(nullptr);\n"
                      "    foo2(NULL);\n"
                      "}");

        ASSERT_EQUALS("", errout.str());

        const Token *f = Token::findsimplematch(tokenizer.tokens(), "foo ( ip ) ;");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 1);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( cip ) ;");
        ASSERT_EQUALS(true, f && f->function() && f->function()->tokenDef->linenr() == 2);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( cp ) ;");
        ASSERT_EQUALS(true, f && f->function() && f->function()->tokenDef->linenr() == 3);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( ccp ) ;");
        ASSERT_EQUALS(true, f && f->function() == nullptr);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( f ) ;");
        ASSERT_EQUALS(true, f && f->function() && f->function()->tokenDef->linenr() == 4);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( b ) ;");
        ASSERT_EQUALS(true, f && f->function() && f->function()->tokenDef->linenr() == 5);

        f = Token::findsimplematch(tokenizer.tokens(), "foo2 ( 0 ) ;");
        ASSERT_EQUALS(true, f && f->function() && f->function()->tokenDef->linenr() == 6);

        f = Token::findsimplematch(tokenizer.tokens(), "foo2 ( nullptr ) ;");
        ASSERT_EQUALS(true, f && f->function() && f->function()->tokenDef->linenr() == 6);

        f = Token::findsimplematch(tokenizer.tokens(), "foo2 ( NULL ) ;");
        ASSERT_EQUALS(true, f && f->function() && f->function()->tokenDef->linenr() == 6);
    }

    void findFunction15() {
        GET_SYMBOL_DB("void foo1(int, char* a) { }\n"
                      "void foo1(int, char a) { }\n"
                      "void foo1(int, wchar_t a) { }\n"
                      "void foo2(int, float a) { }\n"
                      "void foo2(int, wchar_t a) { }\n"
                      "void foo3(int, float a) { }\n"
                      "void foo3(int, char a) { }\n"
                      "void func() {\n"
                      "    foo1(1, 'c');\n"
                      "    foo1(2, L'c');\n"
                      "    foo2(3, 'c');\n"
                      "    foo2(4, L'c');\n"
                      "    foo3(5, 'c');\n"
                      "    foo3(6, L'c');\n"
                      "}");

        ASSERT_EQUALS("", errout.str());

        const Token *f = Token::findsimplematch(tokenizer.tokens(), "foo1 ( 1");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 2);

        f = Token::findsimplematch(tokenizer.tokens(), "foo1 ( 2");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 3);

        f = Token::findsimplematch(tokenizer.tokens(), "foo2 ( 3");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 5);

        f = Token::findsimplematch(tokenizer.tokens(), "foo2 ( 4");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 5);

        f = Token::findsimplematch(tokenizer.tokens(), "foo3 ( 5");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 7);

        f = Token::findsimplematch(tokenizer.tokens(), "foo3 ( 6");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 7);
    }

    void findFunction16() {
        GET_SYMBOL_DB("struct C { int i; static int si; float f; int* ip; float* fp};\n"
                      "void foo(float a) { }\n"
                      "void foo(int a) { }\n"
                      "void foo(int* a) { }\n"
                      "void func(C c, C* cp) {\n"
                      "    foo(c.i);\n"
                      "    foo(cp->i);\n"
                      "    foo(c.f);\n"
                      "    foo(c.si);\n"
                      "    foo(C::si);\n"
                      "    foo(c.ip);\n"
                      "    foo(c.fp);\n"
                      "}");

        ASSERT_EQUALS("", errout.str());

        const Token *f = Token::findsimplematch(tokenizer.tokens(), "foo ( c . i ) ;");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 3);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( cp . i ) ;");
        ASSERT_EQUALS(true, f && f->function() && f->function()->tokenDef->linenr() == 3);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( c . f ) ;");
        ASSERT_EQUALS(true, f && f->function() && f->function()->tokenDef->linenr() == 2);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( c . si ) ;");
        ASSERT_EQUALS(true, f && f->function() && f->function()->tokenDef->linenr() == 3);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( C :: si ) ;");
        ASSERT_EQUALS(true, f && f->function() && f->function()->tokenDef->linenr() == 3);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( c . ip ) ;");
        ASSERT_EQUALS(true, f && f->function() && f->function()->tokenDef->linenr() == 4);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( c . fp ) ;");
        ASSERT_EQUALS(true, f && f->function() == nullptr);
    }

    void findFunction17() {
        GET_SYMBOL_DB("void foo(int a) { }\n"
                      "void foo(float a) { }\n"
                      "void foo(void* a) { }\n"
                      "void foo(bool a) { }\n"
                      "void func(int i, float f, bool b) {\n"
                      "    foo(i + i);\n"
                      "    foo(f + f);\n"
                      "    foo(!b);\n"
                      "    foo(i > 0);\n"
                      "    foo(f + i);\n"
                      "}");

        ASSERT_EQUALS("", errout.str());

        const Token *f = Token::findsimplematch(tokenizer.tokens(), "foo ( i + i ) ;");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 1);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( f + f ) ;");
        ASSERT_EQUALS(true, f && f->function() && f->function()->tokenDef->linenr() == 2);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( ! b ) ;");
        ASSERT_EQUALS(true, f && f->function() && f->function()->tokenDef->linenr() == 4);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( i > 0 ) ;");
        ASSERT_EQUALS(true, f && f->function() && f->function()->tokenDef->linenr() == 4);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( f + i ) ;");
        ASSERT_EQUALS(true, f && f->function() && f->function()->tokenDef->linenr() == 2);
    }

    void findFunction18() {
        GET_SYMBOL_DB("class Fred {\n"
                      "    void f(int i) { }\n"
                      "    void f(float f) const { }\n"
                      "    void a() { f(1); }\n"
                      "    void b() { f(1.f); }\n"
                      "};");

        ASSERT_EQUALS("", errout.str());

        const Token *f = Token::findsimplematch(tokenizer.tokens(), "f ( 1 ) ;");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 2);

        f = Token::findsimplematch(tokenizer.tokens(), "f ( 1.f ) ;");
        ASSERT_EQUALS(true, f && f->function() && f->function()->tokenDef->linenr() == 3);
    }

    void findFunction19() {
        GET_SYMBOL_DB("class Fred {\n"
                      "    enum E1 { e1 };\n"
                      "    enum class E2 : unsigned short { e2 };\n"
                      "    bool               get(bool x) { return x; }\n"
                      "    char               get(char x) { return x; }\n"
                      "    short              get(short x) { return x; }\n"
                      "    int                get(int x) { return x; }\n"
                      "    long               get(long x) { return x; }\n"
                      "    long long          get(long long x) { return x; }\n"
                      "    unsigned char      get(unsigned char x) { return x; }\n"
                      "    signed char        get(signed char x) { return x; }\n"
                      "    unsigned short     get(unsigned short x) { return x; }\n"
                      "    unsigned int       get(unsigned int x) { return x; }\n"
                      "    unsigned long      get(unsigned long x) { return x; }\n"
                      "    unsigned long long get(unsigned long long x) { return x; }\n"
                      "    E1                 get(E1 x) { return x; }\n"
                      "    E2                 get(E2 x) { return x; }\n"
                      "    void foo() {\n"
                      "        bool               v1  = true;   v1  = get(get(v1));\n"
                      "        char               v2  = '1';    v2  = get(get(v2));\n"
                      "        short              v3  = 1;      v3  = get(get(v3));\n"
                      "        int                v4  = 1;      v4  = get(get(v4));\n"
                      "        long               v5  = 1;      v5  = get(get(v5));\n"
                      "        long long          v6  = 1;      v6  = get(get(v6));\n"
                      "        unsigned char      v7  = '1';    v7  = get(get(v7));\n"
                      "        signed char        v8  = '1';    v8  = get(get(v8));\n"
                      "        unsigned short     v9  = 1;      v9  = get(get(v9));\n"
                      "        unsigned int       v10 = 1;      v10 = get(get(v10));\n"
                      "        unsigned long      v11 = 1;      v11 = get(get(v11));\n"
                      "        unsigned long long v12 = 1;      v12 = get(get(v12));\n"
                      "        E1                 v13 = e1;     v13 = get(get(v13));\n"
                      "        E2                 v14 = E2::e2; v14 = get(get(v14));\n"
                      "    }\n"
                      "};");

        ASSERT_EQUALS("", errout.str());

        const Token *f = Token::findsimplematch(tokenizer.tokens(), "get ( get ( v1 ) ) ;");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 4);

        f = Token::findsimplematch(tokenizer.tokens(), "get ( get ( v2 ) ) ;");
        if (std::numeric_limits<char>::is_signed)
            ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 5);
        else
            ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 10);

        f = Token::findsimplematch(tokenizer.tokens(), "get ( get ( v3 ) ) ;");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 6);

        f = Token::findsimplematch(tokenizer.tokens(), "get ( get ( v4 ) ) ;");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 7);

        f = Token::findsimplematch(tokenizer.tokens(), "get ( get ( v5 ) ) ;");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 8);

        f = Token::findsimplematch(tokenizer.tokens(), "get ( get ( v6 ) ) ;");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 9);

        f = Token::findsimplematch(tokenizer.tokens(), "get ( get ( v7 ) ) ;");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 10);

        f = Token::findsimplematch(tokenizer.tokens(), "get ( get ( v8 ) ) ;");
        if (std::numeric_limits<char>::is_signed)
            ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 5);
        else
            ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 10);

        f = Token::findsimplematch(tokenizer.tokens(), "get ( get ( v9 ) ) ;");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 12);

        f = Token::findsimplematch(tokenizer.tokens(), "get ( get ( v10 ) ) ;");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 13);

        f = Token::findsimplematch(tokenizer.tokens(), "get ( get ( v11 ) ) ;");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 14);

        f = Token::findsimplematch(tokenizer.tokens(), "get ( get ( v12 ) ) ;");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 15);

        f = Token::findsimplematch(tokenizer.tokens(), "get ( get ( v13 ) ) ;");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 16);

        f = Token::findsimplematch(tokenizer.tokens(), "get ( get ( v14 ) ) ;");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 17);
    }

    void findFunction20() { // # 8280
        GET_SYMBOL_DB("class Foo {\n"
                      "public:\n"
                      "    Foo() : _x(0), _y(0) {}\n"
                      "    Foo(const Foo& f) {\n"
                      "        copy(&f);\n"
                      "    }\n"
                      "    void copy(const Foo* f) {\n"
                      "        _x=f->_x;\n"
                      "        copy(*f);\n"
                      "    }\n"
                      "private:\n"
                      "    void copy(const Foo& f) {\n"
                      "        _y=f._y;\n"
                      "    }\n"
                      "    int _x;\n"
                      "    int _y;\n"
                      "};");

        ASSERT_EQUALS("", errout.str());

        const Token *f = Token::findsimplematch(tokenizer.tokens(), "copy ( & f ) ;");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 7);

        f = Token::findsimplematch(tokenizer.tokens(), "copy ( * f ) ;");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 12);
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

    void noreturnAttributeFunction() {
        GET_SYMBOL_DB("[[noreturn]] void func1();\n"
                      "void func1() { }\n"
                      "[[noreturn]] void func2();\n"
                      "[[noreturn]] void func3() { }\n"
                      "template <class T> [[noreturn]] void func4() { }");
        ASSERT_EQUALS("", errout.str());
        ASSERT_EQUALS(true,  db != nullptr); // not null

        if (db) {
            const Function *func = findFunctionByName("func1", &db->scopeList.front());
            ASSERT_EQUALS(true, func != nullptr);
            if (func)
                ASSERT_EQUALS(true, func->isAttributeNoreturn());
            func = findFunctionByName("func2", &db->scopeList.front());
            ASSERT_EQUALS(true, func != nullptr);
            if (func)
                ASSERT_EQUALS(true, func->isAttributeNoreturn());
            func = findFunctionByName("func3", &db->scopeList.front());
            ASSERT_EQUALS(true, func != nullptr);
            if (func)
                ASSERT_EQUALS(true, func->isAttributeNoreturn());
            func = findFunctionByName("func4", &db->scopeList.front());
            ASSERT_EQUALS(true, func != nullptr);
            if (func)
                ASSERT_EQUALS(true, func->isAttributeNoreturn());
        }
    }

    void varTypesIntegral() {
        GET_SYMBOL_DB("void f() { bool b; char c; unsigned char uc; short s; unsigned short us; int i; unsigned u; unsigned int ui; long l; unsigned long ul; long long ll; }");
        const Variable *b = db->getVariableFromVarId(1);
        ASSERT(b != nullptr);
        if (b) {
            ASSERT_EQUALS("b", b->nameToken()->str());
            ASSERT_EQUALS(false, b->isFloatingType());
        }
        const Variable *c = db->getVariableFromVarId(2);
        ASSERT(c != nullptr);
        if (c) {
            ASSERT_EQUALS("c", c->nameToken()->str());
            ASSERT_EQUALS(false, c->isFloatingType());
        }
        const Variable *uc = db->getVariableFromVarId(3);
        ASSERT(uc != nullptr);
        if (uc) {
            ASSERT_EQUALS("uc", uc->nameToken()->str());
            ASSERT_EQUALS(false, uc->isFloatingType());
        }
        const Variable *s = db->getVariableFromVarId(4);
        ASSERT(s != nullptr);
        if (s) {
            ASSERT_EQUALS("s", s->nameToken()->str());
            ASSERT_EQUALS(false, s->isFloatingType());
        }
        const Variable *us = db->getVariableFromVarId(5);
        ASSERT(us != nullptr);
        if (us) {
            ASSERT_EQUALS("us", us->nameToken()->str());
            ASSERT_EQUALS(false, us->isFloatingType());
        }
        const Variable *i = db->getVariableFromVarId(6);
        ASSERT(i != nullptr);
        if (i) {
            ASSERT_EQUALS("i", i->nameToken()->str());
            ASSERT_EQUALS(false, i->isFloatingType());
        }
        const Variable *u = db->getVariableFromVarId(7);
        ASSERT(u != nullptr);
        if (u) {
            ASSERT_EQUALS("u", u->nameToken()->str());
            ASSERT_EQUALS(false, u->isFloatingType());
        }
        const Variable *ui = db->getVariableFromVarId(8);
        ASSERT(ui != nullptr);
        if (ui) {
            ASSERT_EQUALS("ui", ui->nameToken()->str());
            ASSERT_EQUALS(false, ui->isFloatingType());
        }
        const Variable *l = db->getVariableFromVarId(9);
        ASSERT(l != nullptr);
        if (l) {
            ASSERT_EQUALS("l", l->nameToken()->str());
            ASSERT_EQUALS(false, l->isFloatingType());
        }
        const Variable *ul = db->getVariableFromVarId(10);
        ASSERT(ul != nullptr);
        if (ul) {
            ASSERT_EQUALS("ul", ul->nameToken()->str());
            ASSERT_EQUALS(false, ul->isFloatingType());
        }
        const Variable *ll = db->getVariableFromVarId(11);
        ASSERT(ll != nullptr);
        if (ll) {
            ASSERT_EQUALS("ll", ll->nameToken()->str());
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
                ASSERT_EQUALS(true, f->isFloatingType());
            }
            const Variable *d = db->getVariableFromVarId(2);
            ASSERT(d != nullptr);
            if (d) {
                ASSERT_EQUALS("d", d->nameToken()->str());
                ASSERT_EQUALS(true, d->isFloatingType());
            }
            const Variable *ld = db->getVariableFromVarId(3);
            ASSERT(ld != nullptr);
            if (ld) {
                ASSERT_EQUALS("ld", ld->nameToken()->str());
                ASSERT_EQUALS(true, ld->isFloatingType());
            }
        }
        {
            GET_SYMBOL_DB("void f() { float * f; static const float * scf; }");
            const Variable *f = db->getVariableFromVarId(1);
            ASSERT(f != nullptr);
            if (f) {
                ASSERT_EQUALS("f", f->nameToken()->str());
                ASSERT_EQUALS(true, f->isFloatingType());
                ASSERT_EQUALS(true, f->isArrayOrPointer());
            }
            const Variable *scf = db->getVariableFromVarId(2);
            ASSERT(scf != nullptr);
            if (scf) {
                ASSERT_EQUALS("scf", scf->nameToken()->str());
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
            ASSERT_EQUALS(false, a->isFloatingType());
        }
        const Variable *b = db->getVariableFromVarId(2);
        ASSERT(b != nullptr);
        if (b) {
            ASSERT_EQUALS("b", b->nameToken()->str());
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

    void lambda2() {
        GET_SYMBOL_DB("void func() {\n"
                      "    float y = 0.0f;\n"
                      "    auto lambda = [&]() -> bool\n"
                      "    {\n"
                      "        float x = 1.0f;\n"
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
    // #6298 "stack overflow in Scope::findFunctionInBase (endless recursion)"
    void circularDependencies() {
        check("template<template<class> class E,class D> class C : E<D> {\n"
              "	public:\n"
              "		int f();\n"
              "};\n"
              "class E : C<D,int> {\n"
              "	public:\n"
              "		int f() { return C< ::D,int>::f(); }\n"
              "};\n"
              "int main() {\n"
              "	E c;\n"
              "	c.f();\n"
              "}");
    }

    void executableScopeWithUnknownFunction() {
        GET_SYMBOL_DB("class Fred {\n"
                      "    void foo(const std::string & a = "");\n"
                      "};\n"
                      "Fred::foo(const std::string & b) { }\n");

        ASSERT(db && db->scopeList.size() == 3);
        if (db && db->scopeList.size() == 3) {
            std::list<Scope>::const_iterator scope = db->scopeList.begin();
            ASSERT_EQUALS(Scope::eGlobal, scope->type);
            ++scope;
            ASSERT_EQUALS(Scope::eClass, scope->type);
            const Scope * class_scope = &*scope;
            ++scope;
            ASSERT(class_scope->functionList.size() == 1);
            if (class_scope->functionList.size() == 1) {
                ASSERT(class_scope->functionList.begin()->hasBody());
                ASSERT(class_scope->functionList.begin()->functionScope == &*scope);
            }
        }
    }

    std::string typeOf(const char code[], const char pattern[], const char filename[] = "test.cpp", const Settings *settings = nullptr) {
        Tokenizer tokenizer(settings ? settings : &settings2, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, filename);
        const Token* tok;
        for (tok = tokenizer.list.back(); tok; tok = tok->previous())
            if (Token::simpleMatch(tok, pattern))
                break;
        return tok->valueType() ? tok->valueType()->str() : std::string();
    }

    void valuetype() {
        // stringification
        ASSERT_EQUALS("", ValueType().str());

        Settings s;
        s.int_bit = 16;
        s.long_bit = 32;
        s.long_long_bit = 64;

        // numbers
        ASSERT_EQUALS("signed int", typeOf("1;", "1", "test.c", &s));
        ASSERT_EQUALS("signed int", typeOf("32767;", "32767", "test.c", &s));
        ASSERT_EQUALS("signed long", typeOf("32768;", "32768", "test.c", &s));
        ASSERT_EQUALS("unsigned int", typeOf("32768U;", "32768U", "test.c", &s));
        ASSERT_EQUALS("signed long long", typeOf("2147483648;", "2147483648", "test.c", &s));
        ASSERT_EQUALS("unsigned int", typeOf("1U;", "1U"));
        ASSERT_EQUALS("signed long", typeOf("1L;", "1L"));
        ASSERT_EQUALS("unsigned long", typeOf("1UL;", "1UL"));
        ASSERT_EQUALS("signed long long", typeOf("1LL;", "1LL"));
        ASSERT_EQUALS("unsigned long long", typeOf("1ULL;", "1ULL"));
        ASSERT_EQUALS("unsigned long long", typeOf("1LLU;", "1LLU"));
        ASSERT_EQUALS("signed long long", typeOf("1i64;", "1i64"));
        ASSERT_EQUALS("unsigned long long", typeOf("1ui64;", "1ui64"));
        ASSERT_EQUALS("unsigned int", typeOf("1u;", "1u"));
        ASSERT_EQUALS("signed long", typeOf("1l;", "1l"));
        ASSERT_EQUALS("unsigned long", typeOf("1ul;", "1ul"));
        ASSERT_EQUALS("signed long long", typeOf("1ll;", "1ll"));
        ASSERT_EQUALS("unsigned long long", typeOf("1ull;", "1ull"));
        ASSERT_EQUALS("unsigned long long", typeOf("1llu;", "1llu"));
        ASSERT_EQUALS("float", typeOf("1.0F;", "1.0F"));
        ASSERT_EQUALS("float", typeOf("1.0f;", "1.0f"));
        ASSERT_EQUALS("double", typeOf("1.0;", "1.0"));
        ASSERT_EQUALS("double", typeOf("1E3;", "1E3"));
        ASSERT_EQUALS("long double", typeOf("1.23L;", "1.23L"));

        // Constant calculations
        ASSERT_EQUALS("signed int", typeOf("1 + 2;", "+"));
        ASSERT_EQUALS("signed long", typeOf("1L + 2;", "+"));
        ASSERT_EQUALS("signed long long", typeOf("1LL + 2;", "+"));
        ASSERT_EQUALS("float", typeOf("1.2f + 3;", "+"));
        ASSERT_EQUALS("float", typeOf("1 + 2.3f;", "+"));

        // promotions
        ASSERT_EQUALS("signed int", typeOf("(char)1 +  (char)2;", "+"));
        ASSERT_EQUALS("signed int", typeOf("(short)1 + (short)2;", "+"));
        ASSERT_EQUALS("signed int", typeOf("(signed int)1 + (signed char)2;", "+"));
        ASSERT_EQUALS("signed int", typeOf("(signed int)1 + (unsigned char)2;", "+"));
        ASSERT_EQUALS("unsigned int", typeOf("(unsigned int)1 + (signed char)2;", "+"));
        ASSERT_EQUALS("unsigned int", typeOf("(unsigned int)1 + (unsigned char)2;", "+"));
        ASSERT_EQUALS("unsigned int", typeOf("(unsigned int)1 + (signed int)2;", "+"));
        ASSERT_EQUALS("unsigned int", typeOf("(unsigned int)1 + (unsigned int)2;", "+"));
        ASSERT_EQUALS("signed long", typeOf("(signed long)1 + (unsigned int)2;", "+"));
        ASSERT_EQUALS("unsigned long", typeOf("(unsigned long)1 + (signed int)2;", "+"));

        // char *
        ASSERT_EQUALS("const char *", typeOf("\"hello\" + 1;", "+"));
        ASSERT_EQUALS("const char",  typeOf("\"hello\"[1];", "["));
        ASSERT_EQUALS("const char",  typeOf(";*\"hello\";", "*"));
        ASSERT_EQUALS("const short *", typeOf("L\"hello\" + 1;", "+"));

        // Variable calculations
        ASSERT_EQUALS("void *", typeOf("void *p; a = p + 1;", "+"));
        ASSERT_EQUALS("signed int", typeOf("int x; a = x + 1;", "+"));
        ASSERT_EQUALS("signed int", typeOf("int x; a = x | 1;", "|"));
        ASSERT_EQUALS("float", typeOf("float x; a = x + 1;", "+"));
        ASSERT_EQUALS("signed int", typeOf("signed x; a = x + 1;", "x +"));
        ASSERT_EQUALS("unsigned int", typeOf("unsigned x; a = x + 1;", "x +"));
        ASSERT_EQUALS("unsigned int", typeOf("unsigned int u1, u2; a = u1 + 1;",  "u1 +"));
        ASSERT_EQUALS("unsigned int", typeOf("unsigned int u1, u2; a = u1 + 1U;", "u1 +"));
        ASSERT_EQUALS("unsigned int", typeOf("unsigned int u1, u2; a = u1 + u2;", "u1 +"));
        ASSERT_EQUALS("unsigned int", typeOf("unsigned int u1, u2; a = u1 * 2;",  "u1 *"));
        ASSERT_EQUALS("unsigned int", typeOf("unsigned int u1, u2; a = u1 * u2;", "u1 *"));
        ASSERT_EQUALS("signed int *", typeOf("int x; a = &x;", "&"));
        ASSERT_EQUALS("signed int *", typeOf("int x; a = &x;", "&"));
        ASSERT_EQUALS("long double", typeOf("long double x; dostuff(x,1);", "x ,"));
        ASSERT_EQUALS("long double *", typeOf("long double x; dostuff(&x,1);", "& x ,"));
        ASSERT_EQUALS("signed int", typeOf("struct X {int i;}; void f(struct X x) { x.i }", "."));
        ASSERT_EQUALS("signed int *", typeOf("int *p; a = p++;", "++"));
        ASSERT_EQUALS("signed int", typeOf("int x; a = x++;", "++"));
        ASSERT_EQUALS("signed int *", typeOf("enum AB {A,B}; AB *ab; x=ab+2;", "+"));
        ASSERT_EQUALS("signed int *", typeOf("enum AB {A,B}; enum AB *ab; x=ab+2;", "+"));
        ASSERT_EQUALS("AB *", typeOf("struct AB {int a; int b;}; AB ab; x=&ab;", "&"));
        ASSERT_EQUALS("AB *", typeOf("struct AB {int a; int b;}; struct AB ab; x=&ab;", "&"));
        ASSERT_EQUALS("A::BC *", typeOf("namespace A { struct BC { int b; int c; }; }; struct A::BC abc; x=&abc;", "&"));
        ASSERT_EQUALS("A::BC *", typeOf("namespace A { struct BC { int b; int c; }; }; struct A::BC *abc; x=abc+1;", "+"));

        // Unary arithmetic/bit operators
        ASSERT_EQUALS("signed int", typeOf("int x; a = -x;", "-"));
        ASSERT_EQUALS("signed int", typeOf("int x; a = ~x;", "~"));
        ASSERT_EQUALS("double", typeOf("double x; a = -x;", "-"));

        // Ternary operator
        ASSERT_EQUALS("signed int", typeOf("int x; a = (b ? x : x);", "?"));
        ASSERT_EQUALS("", typeOf("int x; a = (b ? x : y);", "?"));
        ASSERT_EQUALS("double", typeOf("int x; double y; a = (b ? x : y);", "?"));
        ASSERT_EQUALS("const char *", typeOf("int x; double y; a = (b ? \"a\" : \"b\");", "?"));
        ASSERT_EQUALS("", typeOf("int x; double y; a = (b ? \"a\" : std::string(\"b\"));", "?"));

        // Boolean operators
        ASSERT_EQUALS("bool", typeOf("a > b;", ">"));
        ASSERT_EQUALS("bool", typeOf(";!b;", "!"));
        ASSERT_EQUALS("bool", typeOf("c = a && b;", "&&"));

        // shift => result has same type as lhs
        ASSERT_EQUALS("signed int", typeOf("int x; a = x << 1U;", "<<"));
        ASSERT_EQUALS("signed int", typeOf("int x; a = x >> 1U;", ">>"));
        ASSERT_EQUALS("",           typeOf("a = 12 >> x;", ">>", "test.cpp")); // >> might be overloaded
        ASSERT_EQUALS("signed int", typeOf("a = 12 >> x;", ">>", "test.c"));
        ASSERT_EQUALS("",           typeOf("a = 12 << x;", "<<", "test.cpp")); // << might be overloaded
        ASSERT_EQUALS("signed int", typeOf("a = 12 << x;", "<<", "test.c"));

        // assignment => result has same type as lhs
        ASSERT_EQUALS("unsigned short", typeOf("unsigned short x; x = 3;", "="));

        // array..
        ASSERT_EQUALS("void * *", typeOf("void * x[10]; a = x + 0;", "+"));
        ASSERT_EQUALS("signed int *", typeOf("int x[10]; a = x + 1;", "+"));
        ASSERT_EQUALS("signed int",  typeOf("int x[10]; a = x[0] + 1;", "+"));
        ASSERT_EQUALS("",            typeOf("a = x[\"hello\"];", "[", "test.cpp"));
        ASSERT_EQUALS("const char",  typeOf("a = x[\"hello\"];", "[", "test.c"));

        // cast..
        ASSERT_EQUALS("void *", typeOf("a = (void *)0;", "("));
        ASSERT_EQUALS("char", typeOf("a = (char)32;", "("));
        ASSERT_EQUALS("signed long", typeOf("a = (long)32;", "("));
        ASSERT_EQUALS("signed long", typeOf("a = (long int)32;", "("));
        ASSERT_EQUALS("signed long long", typeOf("a = (long long)32;", "("));
        ASSERT_EQUALS("long double", typeOf("a = (long double)32;", "("));
        ASSERT_EQUALS("char", typeOf("a = static_cast<char>(32);", "("));
        ASSERT_EQUALS("", typeOf("a = (unsigned x)0;", "("));

        // sizeof..
        ASSERT_EQUALS("char", typeOf("sizeof(char);", "char"));

        // const..
        ASSERT_EQUALS("const char *", typeOf("a = \"123\";", "\"123\""));
        ASSERT_EQUALS("const signed int *", typeOf("const int *a; x = a + 1;", "a +"));
        ASSERT_EQUALS("signed int * const", typeOf("int * const a; x = a + 1;", "+"));
        ASSERT_EQUALS("const signed int *", typeOf("const int a[20]; x = a + 1;", "+"));
        ASSERT_EQUALS("const signed int *", typeOf("const int x; a = &x;", "&"));
        ASSERT_EQUALS("signed int", typeOf("int * const a; x = *a;", "*"));
        ASSERT_EQUALS("const signed int", typeOf("const int * const a; x = *a;", "*"));

        // function call..
        ASSERT_EQUALS("signed int", typeOf("int a(int); a(5);", "( 5"));
        ASSERT_EQUALS("signed int", typeOf("auto a(int) -> int; a(5);", "( 5"));
        ASSERT_EQUALS("unsigned long", typeOf("sizeof(x);", "("));
        ASSERT_EQUALS("signed int", typeOf("int (*a)(int); a(5);", "( 5"));

        // struct member..
        ASSERT_EQUALS("signed int", typeOf("struct AB { int a; int b; } ab; x = ab.a;", "."));
        ASSERT_EQUALS("signed int", typeOf("struct AB { int a; int b; } *ab; x = ab[1].a;", "."));

        // Overloaded operators
        ASSERT_EQUALS("Fred", typeOf("class Fred { Fred& operator<(int); }; void f() { Fred fred; x=fred<123; }", "<"));

        // Static members
        ASSERT_EQUALS("signed int", typeOf("struct AB { static int a; }; x = AB::a;", "::"));

        // Pointer to unknown type
        ASSERT_EQUALS("*", typeOf("Bar* b;", "b"));

        // Enum
        ASSERT_EQUALS("char", typeOf("enum E : char { }; void foo() { E e[3]; bar(e[0]); }", "[ 0"));
        ASSERT_EQUALS("signed char", typeOf("enum E : signed char { }; void foo() { E e[3]; bar(e[0]); }", "[ 0"));
        ASSERT_EQUALS("unsigned char", typeOf("enum E : unsigned char { }; void foo() { E e[3]; bar(e[0]); }", "[ 0"));
        ASSERT_EQUALS("signed short", typeOf("enum E : short { }; void foo() { E e[3]; bar(e[0]); }", "[ 0"));
        ASSERT_EQUALS("unsigned short", typeOf("enum E : unsigned short { }; void foo() { E e[3]; bar(e[0]); }", "[ 0"));
        ASSERT_EQUALS("signed int", typeOf("enum E : int { }; void foo() { E e[3]; bar(e[0]); }", "[ 0"));
        ASSERT_EQUALS("unsigned int", typeOf("enum E : unsigned int { }; void foo() { E e[3]; bar(e[0]); }", "[ 0"));
        ASSERT_EQUALS("signed long", typeOf("enum E : long { }; void foo() { E e[3]; bar(e[0]); }", "[ 0"));
        ASSERT_EQUALS("unsigned long", typeOf("enum E : unsigned long { }; void foo() { E e[3]; bar(e[0]); }", "[ 0"));
        ASSERT_EQUALS("signed long long", typeOf("enum E : long long { }; void foo() { E e[3]; bar(e[0]); }", "[ 0"));
        ASSERT_EQUALS("unsigned long long", typeOf("enum E : unsigned long long { }; void foo() { E e[3]; bar(e[0]); }", "[ 0"));

        // Library types
        {
            // PodType
            Settings settingsWin64;
            settingsWin64.platformType = Settings::Win64;
            const Library::PodType u32 = { 4, 'u' };
            settingsWin64.library.mPodTypes["u32"] = u32;
            settingsWin64.library.mPodTypes["xyz::x"] = u32;
            ValueType vt;
            ASSERT_EQUALS(true, vt.fromLibraryType("u32", &settingsWin64));
            ASSERT_EQUALS(true, vt.fromLibraryType("xyz::x", &settingsWin64));
            ASSERT_EQUALS(ValueType::Type::INT, vt.type);
            ASSERT_EQUALS("unsigned int *", typeOf(";void *data = new u32[10];", "new", "test.cpp", &settingsWin64));
            ASSERT_EQUALS("unsigned int *", typeOf(";void *data = new xyz::x[10];", "new", "test.cpp", &settingsWin64));
            ASSERT_EQUALS("unsigned int", typeOf("; x = (xyz::x)12;", "(", "test.cpp", &settingsWin64));
        }
        {
            // PlatformType
            Settings settingsUnix32;
            settingsUnix32.platformType = Settings::Unix32;
            Library::PlatformType s32;
            s32.mType = "int";
            settingsUnix32.library.mPlatforms[settingsUnix32.platformString()].mPlatformTypes["s32"] = s32;
            ValueType vt;
            ASSERT_EQUALS(true, vt.fromLibraryType("s32", &settingsUnix32));
            ASSERT_EQUALS(ValueType::Type::INT, vt.type);
        }
        {
            // Container
            Settings sC;
            Library::Container c;
            c.startPattern = "C";
            sC.library.containers["C"] = c;
            ASSERT_EQUALS("container(C) *", typeOf("C*c=new C;","new","test.cpp",&sC));
            ASSERT_EQUALS("container(C) *", typeOf("x=(C*)c;","(","test.cpp",&sC));
        }
        {
            // Container (vector)
            Settings set;
            Library::Container vector;
            vector.startPattern = "Vector";
            vector.type_templateArgNo = 0;
            vector.arrayLike_indexOp = true;
            set.library.containers["Vector"] = vector;
            ASSERT_EQUALS("signed int", typeOf("Vector<int> v; v[0]=3;", "[", "test.cpp", &set));
        }

        // new
        ASSERT_EQUALS("C *", typeOf("class C {}; x = new C();", "new"));

        // auto variables
        ASSERT_EQUALS("signed int", typeOf("; auto x = 3;", "x"));
        ASSERT_EQUALS("signed int *", typeOf("; auto *p = (int *)0;", "p"));
        ASSERT_EQUALS("signed int *", typeOf("; auto data = new int[100];", "data"));
        ASSERT_EQUALS("signed int", typeOf("; auto data = new X::Y; int x=1000; x=x/5;", "/")); // #7970
        ASSERT_EQUALS("signed int *", typeOf("; auto data = new (nothrow) int[100];", "data"));
        ASSERT_EQUALS("signed int *", typeOf("; auto data = new (std::nothrow) int[100];", "data"));
        ASSERT_EQUALS("const signed short", typeOf("short values[10]; void f() { for (const auto *x : values); }", "x"));
        ASSERT_EQUALS("signed int *", typeOf("MACRO(test) void test() { auto x = (int*)y; }", "x")); // #7931 (garbage?)

        // Variable declaration
        ASSERT_EQUALS("char *", typeOf("; char abc[] = \"abc\";", "["));
        ASSERT_EQUALS("", typeOf("; int x[10] = { [3]=1 };", "[ 3 ]"));
    }

    void variadic1() { // #7453
        {
            GET_SYMBOL_DB("CBase* create(const char *c1, ...);\n"
                          "int    create(COther& ot, const char *c1, ...);\n"
                          "int foo(COther & ot)\n"
                          "{\n"
                          "   CBase* cp1 = create(\"AAAA\", 44, (char*)0);\n"
                          "   CBase* cp2 = create(ot, \"AAAA\", 44, (char*)0);\n"
                          "}");

            const Token *f = Token::findsimplematch(tokenizer.tokens(), "create ( \"AAAA\"");
            ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 1);
            f = Token::findsimplematch(tokenizer.tokens(), "create ( ot");
            ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 2);
        }
        {
            GET_SYMBOL_DB("int    create(COther& ot, const char *c1, ...);\n"
                          "CBase* create(const char *c1, ...);\n"
                          "int foo(COther & ot)\n"
                          "{\n"
                          "   CBase* cp1 = create(\"AAAA\", 44, (char*)0);\n"
                          "   CBase* cp2 = create(ot, \"AAAA\", 44, (char*)0);\n"
                          "}");

            const Token *f = Token::findsimplematch(tokenizer.tokens(), "create ( \"AAAA\"");
            ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 2);
            f = Token::findsimplematch(tokenizer.tokens(), "create ( ot");
            ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 1);
        }
    }

    void variadic2() { // #7649
        {
            GET_SYMBOL_DB("CBase* create(const char *c1, ...);\n"
                          "CBase* create(const wchar_t *c1, ...);\n"
                          "int foo(COther & ot)\n"
                          "{\n"
                          "   CBase* cp1 = create(\"AAAA\", 44, (char*)0);\n"
                          "   CBase* cp2 = create(L\"AAAA\", 44, (char*)0);\n"
                          "}");

            const Token *f = Token::findsimplematch(tokenizer.tokens(), "cp1 = create (");
            ASSERT_EQUALS(true, db && f && f->tokAt(2) && f->tokAt(2)->function() && f->tokAt(2)->function()->tokenDef->linenr() == 1);
            f = Token::findsimplematch(tokenizer.tokens(), "cp2 = create (");
            ASSERT_EQUALS(true, db && f && f->tokAt(2) && f->tokAt(2)->function() && f->tokAt(2)->function()->tokenDef->linenr() == 2);
        }
        {
            GET_SYMBOL_DB("CBase* create(const wchar_t *c1, ...);\n"
                          "CBase* create(const char *c1, ...);\n"
                          "int foo(COther & ot)\n"
                          "{\n"
                          "   CBase* cp1 = create(\"AAAA\", 44, (char*)0);\n"
                          "   CBase* cp2 = create(L\"AAAA\", 44, (char*)0);\n"
                          "}");

            const Token *f = Token::findsimplematch(tokenizer.tokens(), "cp1 = create (");
            ASSERT_EQUALS(true, db && f && f->tokAt(2) && f->tokAt(2)->function() && f->tokAt(2)->function()->tokenDef->linenr() == 2);
            f = Token::findsimplematch(tokenizer.tokens(), "cp2 = create (");
            ASSERT_EQUALS(true, db && f && f->tokAt(2) && f->tokAt(2)->function() && f->tokAt(2)->function()->tokenDef->linenr() == 1);
        }
    }

    void variadic3() { // #7387
        {
            GET_SYMBOL_DB("int zdcalc(const XYZ & per, short rs = 0);\n"
                          "double zdcalc(long& length, const XYZ * per);\n"
                          "long mycalc( ) {\n"
                          "    long length;\n"
                          "    XYZ per;\n"
                          "    zdcalc(length, &per);\n"
                          "}");

            const Token *f = Token::findsimplematch(tokenizer.tokens(), "zdcalc ( length");
            ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 2);
        }
        {
            GET_SYMBOL_DB("double zdcalc(long& length, const XYZ * per);\n"
                          "int zdcalc(const XYZ & per, short rs = 0);\n"
                          "long mycalc( ) {\n"
                          "    long length;\n"
                          "    XYZ per;\n"
                          "    zdcalc(length, &per);\n"
                          "}");

            const Token *f = Token::findsimplematch(tokenizer.tokens(), "zdcalc ( length");
            ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 1);
        }
    }

    void noReturnType() {
        GET_SYMBOL_DB_C("func() { }");

        ASSERT(db && db->functionScopes.size() == 1);
        if (db && db->functionScopes.size() == 1) {
            ASSERT(db->functionScopes[0]->function != nullptr);
            if (db->functionScopes[0]->function != nullptr) {
                const Token *retDef = db->functionScopes[0]->function->retDef;
                ASSERT_EQUALS("func", retDef ? retDef->str() : "");
            }
        }
    }

    void auto1() {
        GET_SYMBOL_DB("; auto x = \"abc\";");
        const Token *autotok = tokenizer.tokens()->next();
        ASSERT(autotok && autotok->isStandardType());
        const Variable *var = db ? db->getVariableFromVarId(1) : nullptr;
        ASSERT(var && var->isPointer() && !var->isConst());
    }

    void auto2() {
        GET_SYMBOL_DB("struct S { int i; };\n"
                      "int foo() {\n"
                      "    auto a = new S;\n"
                      "    auto * b = new S;\n"
                      "    auto c = new S[10];\n"
                      "    auto * d = new S[10];\n"
                      "    return a->i + b->i + c[0]->i + d[0]->i;\n"
                      "}");
        const Token *autotok = Token::findsimplematch(tokenizer.tokens(), "auto");
        ASSERT(db && autotok && autotok->valueType() && autotok->valueType()->pointer == 1 && autotok->valueType()->typeScope && autotok->valueType()->typeScope->definedType && autotok->valueType()->typeScope->definedType->name() == "S" && autotok->type() == nullptr);

        autotok = Token::findsimplematch(autotok->next(), "auto");
        ASSERT(db && autotok && autotok->valueType() && autotok->valueType()->pointer == 0 && autotok->valueType()->typeScope && autotok->valueType()->typeScope->definedType && autotok->valueType()->typeScope->definedType->name() == "S" && autotok->type() && autotok->type()->name() == "S");

        autotok = Token::findsimplematch(autotok->next(), "auto");
        ASSERT(db && autotok && autotok->valueType() && autotok->valueType()->pointer == 1 && autotok->valueType()->typeScope && autotok->valueType()->typeScope->definedType && autotok->valueType()->typeScope->definedType->name() == "S" && autotok->type() == nullptr);

        autotok = Token::findsimplematch(autotok->next(), "auto");
        ASSERT(db && autotok && autotok->valueType() && autotok->valueType()->pointer == 0 && autotok->valueType()->typeScope && autotok->valueType()->typeScope->definedType && autotok->valueType()->typeScope->definedType->name() == "S" && autotok->type() && autotok->type()->name() == "S");

        vartok = Token::findsimplematch(tokenizer.tokens(), "a =");
        ASSERT(db && vartok && vartok->variable() && vartok->variable()->isPointer() && vartok->variable()->type() && vartok->variable()->type()->name() == "S");

        vartok = Token::findsimplematch(vartok->next(), "b =");
        ASSERT(db && vartok && vartok->variable() && vartok->variable()->isPointer() && vartok->variable()->type() && vartok->variable()->type()->name() == "S");

        vartok = Token::findsimplematch(vartok->next(), "c =");
        ASSERT(db && vartok && vartok->variable() && vartok->variable()->isPointer() && vartok->variable()->type() && vartok->variable()->type()->name() == "S");

        vartok = Token::findsimplematch(vartok->next(), "d =");
        ASSERT(db && vartok && vartok->variable() && vartok->variable()->isPointer() && vartok->variable()->type() && vartok->variable()->type()->name() == "S");

        vartok = Token::findsimplematch(tokenizer.tokens(), "return");

        vartok = Token::findsimplematch(vartok, "a");
        ASSERT(db && vartok && vartok->variable() && vartok->variable()->isPointer() && vartok->variable()->type() && vartok->variable()->type()->name() == "S");

        vartok = Token::findsimplematch(vartok->next(), "b");
        ASSERT(db && vartok && vartok->variable() && vartok->variable()->isPointer() && vartok->variable()->type() && vartok->variable()->type()->name() == "S");

        vartok = Token::findsimplematch(vartok->next(), "c");
        ASSERT(db && vartok && vartok->variable() && vartok->variable()->isPointer() && vartok->variable()->type() && vartok->variable()->type()->name() == "S");

        vartok = Token::findsimplematch(vartok->next(), "d");
        ASSERT(db && vartok && vartok->variable() && vartok->variable()->isPointer() && vartok->variable()->type() && vartok->variable()->type()->name() == "S");

        vartok = Token::findsimplematch(tokenizer.tokens(), "return");

        vartok = Token::findsimplematch(vartok, "i");
        ASSERT(db && vartok && vartok->variable() && vartok->variable()->typeStartToken()->str() == "int");

        vartok = Token::findsimplematch(vartok->next(), "i");
        ASSERT(db && vartok && vartok->variable() && vartok->variable()->typeStartToken()->str() == "int");

        vartok = Token::findsimplematch(vartok->next(), "i");
        ASSERT(db && vartok && vartok->variable() && vartok->variable()->typeStartToken()->str() == "int");

        vartok = Token::findsimplematch(vartok->next(), "i");
        ASSERT(db && vartok && vartok->variable() && vartok->variable()->typeStartToken()->str() == "int");
    }

    void auto3() {
        GET_SYMBOL_DB("enum E : unsigned short { A, B, C };\n"
                      "int foo() {\n"
                      "    auto a = new E;\n"
                      "    auto * b = new E;\n"
                      "    auto c = new E[10];\n"
                      "    auto * d = new E[10];\n"
                      "    return *a + *b + c[0] + d[0];\n"
                      "}");
        const Token *autotok = Token::findsimplematch(tokenizer.tokens(), "auto");
        ASSERT(db && autotok && autotok->valueType() && autotok->valueType()->pointer == 1 && autotok->valueType()->typeScope && autotok->valueType()->typeScope->definedType && autotok->valueType()->typeScope->definedType->name() == "E" && autotok->type() == nullptr);

        autotok = Token::findsimplematch(autotok->next(), "auto");
        ASSERT(db && autotok && autotok->valueType() && autotok->valueType()->pointer == 0 && autotok->valueType()->typeScope && autotok->valueType()->typeScope->definedType && autotok->valueType()->typeScope->definedType->name() == "E" && autotok->type() && autotok->type()->name() == "E");

        autotok = Token::findsimplematch(autotok->next(), "auto");
        ASSERT(db && autotok && autotok->valueType() && autotok->valueType()->pointer == 1 && autotok->valueType()->typeScope && autotok->valueType()->typeScope->definedType && autotok->valueType()->typeScope->definedType->name() == "E" && autotok->type() == nullptr);

        autotok = Token::findsimplematch(autotok->next(), "auto");
        ASSERT(db && autotok && autotok->valueType() && autotok->valueType()->pointer == 0 && autotok->valueType()->typeScope && autotok->valueType()->typeScope->definedType && autotok->valueType()->typeScope->definedType->name() == "E" && autotok->type() && autotok->type()->name() == "E");

        vartok = Token::findsimplematch(tokenizer.tokens(), "a =");
        ASSERT(db && vartok && vartok->variable() && vartok->variable()->isPointer() && vartok->variable()->type() && vartok->variable()->type()->name() == "E");

        vartok = Token::findsimplematch(vartok->next(), "b =");
        ASSERT(db && vartok && vartok->variable() && vartok->variable()->isPointer() && vartok->variable()->type() && vartok->variable()->type()->name() == "E");

        vartok = Token::findsimplematch(vartok->next(), "c =");
        ASSERT(db && vartok && vartok->variable() && vartok->variable()->isPointer() && vartok->variable()->type() && vartok->variable()->type()->name() == "E");

        vartok = Token::findsimplematch(vartok->next(), "d =");
        ASSERT(db && vartok && vartok->variable() && vartok->variable()->isPointer() && vartok->variable()->type() && vartok->variable()->type()->name() == "E");

        vartok = Token::findsimplematch(tokenizer.tokens(), "return");

        vartok = Token::findsimplematch(vartok, "a");
        ASSERT(db && vartok && vartok->variable() && vartok->variable()->isPointer() && vartok->variable()->type() && vartok->variable()->type()->name() == "E");

        vartok = Token::findsimplematch(vartok->next(), "b");
        ASSERT(db && vartok && vartok->variable() && vartok->variable()->isPointer() && vartok->variable()->type() && vartok->variable()->type()->name() == "E");

        vartok = Token::findsimplematch(vartok->next(), "c");
        ASSERT(db && vartok && vartok->variable() && vartok->variable()->isPointer() && vartok->variable()->type() && vartok->variable()->type()->name() == "E");

        vartok = Token::findsimplematch(vartok->next(), "d");
        ASSERT(db && vartok && vartok->variable() && vartok->variable()->isPointer() && vartok->variable()->type() && vartok->variable()->type()->name() == "E");
    }

    void auto4() {
        GET_SYMBOL_DB("struct S { int i; };\n"
                      "int foo() {\n"
                      "    S array[10];\n"
                      "    for (auto a : array)\n"
                      "        a.i = 0;\n"
                      "    for (auto & b : array)\n"
                      "        b.i = 1;\n"
                      "    for (const auto & c : array)\n"
                      "        auto ci = c.i;\n"
                      "    for (auto * d : array)\n"
                      "        d->i = 0;\n"
                      "    for (const auto * e : array)\n"
                      "        auto ei = e->i;\n"
                      "    return array[0].i;\n"
                      "}");
        const Token *autotok = Token::findsimplematch(tokenizer.tokens(), "auto a");
        ASSERT(db && autotok && autotok->valueType() && autotok->valueType()->pointer == 0 && autotok->valueType()->constness == 0 && autotok->valueType()->typeScope && autotok->valueType()->typeScope->definedType && autotok->valueType()->typeScope->definedType->name() == "S");
        ASSERT(db && autotok && autotok->type() && autotok->type()->name() == "S");

        autotok = Token::findsimplematch(autotok->next(), "auto & b");
        ASSERT(db && autotok && autotok->valueType() && autotok->valueType()->pointer == 0 && autotok->valueType()->constness == 0 && autotok->valueType()->typeScope && autotok->valueType()->typeScope->definedType && autotok->valueType()->typeScope->definedType->name() == "S");
        ASSERT(db && autotok && autotok->type() && autotok->type()->name() == "S");

        autotok = Token::findsimplematch(autotok->next(), "auto & c");
        ASSERT(db && autotok && autotok->valueType() && autotok->valueType()->pointer == 0 && autotok->valueType()->constness == 0 && autotok->valueType()->typeScope && autotok->valueType()->typeScope->definedType && autotok->valueType()->typeScope->definedType->name() == "S");
        ASSERT(db && autotok && autotok->type() && autotok->type()->name() == "S");

        autotok = Token::findsimplematch(autotok->next(), "auto * d");
        ASSERT(db && autotok && autotok->valueType() && autotok->valueType()->pointer == 0 && autotok->valueType()->constness == 0 && autotok->valueType()->typeScope && autotok->valueType()->typeScope->definedType && autotok->valueType()->typeScope->definedType->name() == "S");
        ASSERT(db && autotok && autotok->type() && autotok->type()->name() == "S");

        autotok = Token::findsimplematch(autotok->next(), "auto * e");
        ASSERT(db && autotok && autotok->valueType() && autotok->valueType()->pointer == 0 && autotok->valueType()->constness == 0 && autotok->valueType()->typeScope && autotok->valueType()->typeScope->definedType && autotok->valueType()->typeScope->definedType->name() == "S");
        ASSERT(db && autotok && autotok->type() && autotok->type()->name() == "S");

        vartok = Token::findsimplematch(tokenizer.tokens(), "a :");
        ASSERT(db && vartok && vartok->valueType() && vartok->valueType()->typeScope && vartok->valueType()->typeScope->definedType && vartok->valueType()->typeScope->definedType->name() == "S");
        ASSERT(db && vartok && vartok->variable() && !vartok->variable()->isReference() && !vartok->variable()->isPointer());
        ASSERT(db && vartok && vartok->variable() && vartok->variable()->type() && vartok->variable()->type()->name() == "S");

        vartok = Token::findsimplematch(vartok->next(), "b :");
        ASSERT(db && vartok && vartok->valueType() && vartok->valueType()->typeScope && vartok->valueType()->typeScope->definedType && vartok->valueType()->typeScope->definedType->name() == "S");
        ASSERT(db && vartok && vartok->variable() && vartok->variable()->isReference() && !vartok->variable()->isPointer() && !vartok->variable()->isConst());

        vartok = Token::findsimplematch(vartok->next(), "c :");
        ASSERT(db && vartok && vartok->valueType() && vartok->valueType()->typeScope && vartok->valueType()->typeScope->definedType && vartok->valueType()->typeScope->definedType->name() == "S");
        ASSERT(db && vartok && vartok->variable() && vartok->variable()->isReference() && !vartok->variable()->isPointer() && vartok->variable()->isConst());

        vartok = Token::findsimplematch(vartok->next(), "d :");
        ASSERT(db && vartok && vartok->valueType() && vartok->valueType()->typeScope && vartok->valueType()->typeScope->definedType && vartok->valueType()->typeScope->definedType->name() == "S");
        ASSERT(db && vartok && vartok->variable() && !vartok->variable()->isReference() && vartok->variable()->isPointer() && !vartok->variable()->isConst());

        vartok = Token::findsimplematch(vartok->next(), "e :");
        ASSERT(db && vartok && vartok->valueType() && vartok->valueType()->typeScope && vartok->valueType()->typeScope->definedType && vartok->valueType()->typeScope->definedType->name() == "S");
        ASSERT(db && vartok && vartok->variable() && !vartok->variable()->isReference() && vartok->variable()->isPointer() && vartok->variable()->isConst());

        vartok = Token::findsimplematch(tokenizer.tokens(), "a . i");
        ASSERT(db && vartok && vartok->variable() && !vartok->variable()->isPointer() && !vartok->variable()->isReference() && vartok->variable()->type() && vartok->variable()->type()->name() == "S");

        vartok = Token::findsimplematch(vartok, "i");
        ASSERT(db && vartok && vartok->variable() && vartok->variable()->typeStartToken()->str() == "int");

        vartok = Token::findsimplematch(vartok->next(), "b . i");
        ASSERT(db && vartok && vartok->variable() && !vartok->variable()->isPointer() && vartok->variable()->isReference() && vartok->variable()->type() && vartok->variable()->type()->name() == "S");

        vartok = Token::findsimplematch(vartok->next(), "i");
        ASSERT(db && vartok && vartok->variable() && vartok->variable()->typeStartToken()->str() == "int");

        vartok = Token::findsimplematch(vartok->next(), "c . i");
        ASSERT(db && vartok && vartok->variable() && !vartok->variable()->isPointer() && vartok->variable()->isReference() && vartok->variable()->type() && vartok->variable()->type()->name() == "S");

        vartok = Token::findsimplematch(vartok->next(), "i");
        ASSERT(db && vartok && vartok->variable() && vartok->variable()->typeStartToken()->str() == "int");

        vartok = Token::findsimplematch(vartok->next(), "d . i");
        ASSERT(db && vartok && vartok->variable() && vartok->variable()->isPointer() && !vartok->variable()->isReference() && vartok->variable()->type() && vartok->variable()->type()->name() == "S");

        vartok = Token::findsimplematch(vartok->next(), "i");
        ASSERT(db && vartok && vartok->variable() && vartok->variable()->typeStartToken()->str() == "int");

        vartok = Token::findsimplematch(vartok->next(), "e . i");
        ASSERT(db && vartok && vartok->variable() && vartok->variable()->isPointer() && !vartok->variable()->isReference() && vartok->variable()->type() && vartok->variable()->type()->name() == "S");

        vartok = Token::findsimplematch(vartok->next(), "i");
        ASSERT(db && vartok && vartok->variable() && vartok->variable()->typeStartToken()->str() == "int");

        vartok = Token::findsimplematch(vartok->next(), "i");
        ASSERT(db && vartok && vartok->variable() && vartok->variable()->typeStartToken()->str() == "int");
    }

    void auto5() {
        GET_SYMBOL_DB("struct S { int i; };\n"
                      "int foo() {\n"
                      "    std::vector<S> vec(10);\n"
                      "    for (auto a : vec)\n"
                      "        a.i = 0;\n"
                      "    for (auto & b : vec)\n"
                      "        b.i = 0;\n"
                      "    for (const auto & c : vec)\n"
                      "        auto ci = c.i;\n"
                      "    for (auto * d : vec)\n"
                      "        d.i = 0;\n"
                      "    for (const auto * e : vec)\n"
                      "        auto ei = e->i;\n"
                      "    return vec[0].i;\n"
                      "}");
        const Token *autotok = Token::findsimplematch(tokenizer.tokens(), "auto a");
        ASSERT(db && autotok && autotok->valueType() && autotok->valueType()->pointer == 0 && autotok->valueType()->constness == 0 && autotok->valueType()->typeScope && autotok->valueType()->typeScope->definedType && autotok->valueType()->typeScope->definedType->name() == "S");
        ASSERT(db && autotok && autotok->type() && autotok->type()->name() == "S");

        autotok = Token::findsimplematch(autotok->next(), "auto & b");
        ASSERT(db && autotok && autotok->valueType() && autotok->valueType()->pointer == 0 && autotok->valueType()->constness == 0 && autotok->valueType()->typeScope && autotok->valueType()->typeScope->definedType && autotok->valueType()->typeScope->definedType->name() == "S");
        ASSERT(db && autotok && autotok->type() && autotok->type()->name() == "S");

        autotok = Token::findsimplematch(autotok->next(), "auto & c");
        ASSERT(db && autotok && autotok->valueType() && autotok->valueType()->pointer == 0 && autotok->valueType()->constness == 0 && autotok->valueType()->typeScope && autotok->valueType()->typeScope->definedType && autotok->valueType()->typeScope->definedType->name() == "S");
        ASSERT(db && autotok && autotok->type() && autotok->type()->name() == "S");

        autotok = Token::findsimplematch(autotok->next(), "auto * d");
        ASSERT(db && autotok && autotok->valueType() && autotok->valueType()->pointer == 0 && autotok->valueType()->constness == 0 && autotok->valueType()->typeScope && autotok->valueType()->typeScope->definedType && autotok->valueType()->typeScope->definedType->name() == "S");
        ASSERT(db && autotok && autotok->type() && autotok->type()->name() == "S");

        autotok = Token::findsimplematch(autotok->next(), "auto * e");
        ASSERT(db && autotok && autotok->valueType() && autotok->valueType()->pointer == 0 && autotok->valueType()->constness == 0 && autotok->valueType()->typeScope && autotok->valueType()->typeScope->definedType && autotok->valueType()->typeScope->definedType->name() == "S");
        ASSERT(db && autotok && autotok->type() && autotok->type()->name() == "S");

        vartok = Token::findsimplematch(tokenizer.tokens(), "a :");
        ASSERT(db && vartok && vartok->valueType() && vartok->valueType()->typeScope && vartok->valueType()->typeScope->definedType && vartok->valueType()->typeScope->definedType->name() == "S");
        ASSERT(db && vartok && vartok->variable() && !vartok->variable()->isReference() && !vartok->variable()->isPointer());
        ASSERT(db && vartok && vartok->variable() && vartok->variable()->type() && vartok->variable()->type()->name() == "S");

        vartok = Token::findsimplematch(vartok->next(), "b :");
        ASSERT(db && vartok && vartok->valueType() && vartok->valueType()->typeScope && vartok->valueType()->typeScope->definedType && vartok->valueType()->typeScope->definedType->name() == "S");
        ASSERT(db && vartok && vartok->variable() && vartok->variable()->isReference() && !vartok->variable()->isPointer() && !vartok->variable()->isConst());

        vartok = Token::findsimplematch(vartok->next(), "c :");
        ASSERT(db && vartok && vartok->valueType() && vartok->valueType()->typeScope && vartok->valueType()->typeScope->definedType && vartok->valueType()->typeScope->definedType->name() == "S");
        ASSERT(db && vartok && vartok->variable() && vartok->variable()->isReference() && !vartok->variable()->isPointer() && vartok->variable()->isConst());

        vartok = Token::findsimplematch(vartok->next(), "d :");
        ASSERT(db && vartok && vartok->valueType() && vartok->valueType()->typeScope && vartok->valueType()->typeScope->definedType && vartok->valueType()->typeScope->definedType->name() == "S");
        ASSERT(db && vartok && vartok->variable() && !vartok->variable()->isReference() && vartok->variable()->isPointer() && !vartok->variable()->isConst());

        vartok = Token::findsimplematch(vartok->next(), "e :");
        ASSERT(db && vartok && vartok->valueType() && vartok->valueType()->typeScope && vartok->valueType()->typeScope->definedType && vartok->valueType()->typeScope->definedType->name() == "S");
        ASSERT(db && vartok && vartok->variable() && !vartok->variable()->isReference() && vartok->variable()->isPointer() && vartok->variable()->isConst());


        vartok = Token::findsimplematch(tokenizer.tokens(), "a . i");
        ASSERT(db && vartok && vartok->variable() && !vartok->variable()->isPointer() && !vartok->variable()->isReference() && vartok->variable()->type() && vartok->variable()->type()->name() == "S");

        vartok = Token::findsimplematch(vartok, "i");
        ASSERT(db && vartok && vartok->variable() && vartok->variable()->typeStartToken()->str() == "int");

        vartok = Token::findsimplematch(vartok->next(), "b . i");
        ASSERT(db && vartok && vartok->variable() && !vartok->variable()->isPointer() && vartok->variable()->isReference() && vartok->variable()->type() && vartok->variable()->type()->name() == "S");

        vartok = Token::findsimplematch(vartok->next(), "i");
        ASSERT(db && vartok && vartok->variable() && vartok->variable()->typeStartToken()->str() == "int");

        vartok = Token::findsimplematch(vartok->next(), "c . i");
        ASSERT(db && vartok && vartok->variable() && !vartok->variable()->isPointer() && vartok->variable()->isReference() && vartok->variable()->type() && vartok->variable()->type()->name() == "S");

        vartok = Token::findsimplematch(vartok->next(), "i");
        ASSERT(db && vartok && vartok->variable() && vartok->variable()->typeStartToken()->str() == "int");

        vartok = Token::findsimplematch(vartok->next(), "d . i");
        ASSERT(db && vartok && vartok->variable() && vartok->variable()->isPointer() && !vartok->variable()->isReference() && vartok->variable()->type() && vartok->variable()->type()->name() == "S");

        vartok = Token::findsimplematch(vartok->next(), "i");
        ASSERT(db && vartok && vartok->variable() && vartok->variable()->typeStartToken()->str() == "int");

        vartok = Token::findsimplematch(vartok->next(), "e . i");
        ASSERT(db && vartok && vartok->variable() && vartok->variable()->isPointer() && !vartok->variable()->isReference() && vartok->variable()->type() && vartok->variable()->type()->name() == "S");

        vartok = Token::findsimplematch(vartok->next(), "i");
        ASSERT(db && vartok && vartok->variable() && vartok->variable()->typeStartToken()->str() == "int");

        vartok = Token::findsimplematch(vartok->next(), "i");
        ASSERT(db && vartok && vartok->variable() && vartok->variable()->typeStartToken()->str() == "int");
    }

    void auto6() {  // #7963 (segmentation fault)
        GET_SYMBOL_DB("class WebGLTransformFeedback final\n"
                      ": public nsWrapperCache\n"
                      ", public WebGLRefCountedObject < WebGLTransformFeedback >\n"
                      ", public LinkedListElement < WebGLTransformFeedback >\n"
                      "{\n"
                      "private :\n"
                      "std :: vector < IndexedBufferBinding > mIndexedBindings ;\n"
                      "} ;\n"
                      "struct IndexedBufferBinding\n"
                      "{\n"
                      "IndexedBufferBinding ( ) ;\n"
                      "} ;\n"
                      "const decltype ( WebGLTransformFeedback :: mBuffersForTF ) &\n"
                      "WebGLTransformFeedback :: BuffersForTF ( ) const\n"
                      "{\n"
                      "mBuffersForTF . clear ( ) ;\n"
                      "for ( const auto & cur : mIndexedBindings ) {}\n"
                      "return mBuffersForTF ;\n"
                      "}");
        ASSERT_EQUALS(true,  db != nullptr); // not null
    }

    void auto7() {
        GET_SYMBOL_DB("struct Foo { int a; int b[10]; };\n"
                      "class Bar {\n"
                      "    Foo foo1;\n"
                      "    Foo foo2[10];\n"
                      "public:\n"
                      "    const Foo & getFoo1() { return foo1; }\n"
                      "    const Foo * getFoo2() { return foo2; }\n"
                      "};\n"
                      "int main() {\n"
                      "    Bar bar;\n"
                      "    auto v1 = bar.getFoo1().a;\n"
                      "    auto v2 = bar.getFoo1().b[0];\n"
                      "    auto v3 = bar.getFoo1().b;\n"
                      "    const auto v4 = bar.getFoo1().b;\n"
                      "    const auto * v5 = bar.getFoo1().b;\n"
                      "    auto v6 = bar.getFoo2()[0].a;\n"
                      "    auto v7 = bar.getFoo2()[0].b[0];\n"
                      "    auto v8 = bar.getFoo2()[0].b;\n"
                      "    const auto v9 = bar.getFoo2()[0].b;\n"
                      "    const auto * v10 = bar.getFoo2()[0].b;\n"
                      "    auto v11 = v1 + v2 + v3[0] + v4[0] + v5[0] + v6 + v7 + v8[0] + v9[0] + v10[0];\n"
                      "    return v11;\n"
                      "}");
        const Token *autotok = Token::findsimplematch(tokenizer.tokens(), "auto v1");

        // auto = int, v1 = int
        ASSERT(db && autotok && autotok->valueType());
        if (db && autotok && autotok->valueType()) {
            ASSERT_EQUALS(0, autotok->valueType()->constness);
            ASSERT_EQUALS(0, autotok->valueType()->pointer);
            ASSERT_EQUALS(ValueType::SIGNED, autotok->valueType()->sign);
            ASSERT_EQUALS(ValueType::INT, autotok->valueType()->type);
        }
        vartok = Token::findsimplematch(autotok, "v1 =");
        ASSERT(db && vartok && vartok->valueType());
        if (db && vartok && vartok->valueType()) {
            ASSERT_EQUALS(0, vartok->valueType()->constness);
            ASSERT_EQUALS(0, vartok->valueType()->pointer);
            ASSERT_EQUALS(ValueType::SIGNED, vartok->valueType()->sign);
            ASSERT_EQUALS(ValueType::INT, vartok->valueType()->type);
        }

        // auto = int, v2 = int
        autotok = Token::findsimplematch(autotok, "auto v2");
        ASSERT(db && autotok && autotok->valueType());
        if (db && autotok && autotok->valueType()) {
            ASSERT_EQUALS(0, autotok->valueType()->constness);
            ASSERT_EQUALS(0, autotok->valueType()->pointer);
            ASSERT_EQUALS(ValueType::SIGNED, autotok->valueType()->sign);
            ASSERT_EQUALS(ValueType::INT, autotok->valueType()->type);
        }
        vartok = Token::findsimplematch(autotok, "v2 =");
        ASSERT(db && vartok && vartok->valueType());
        if (db && vartok && vartok->valueType()) {
            ASSERT_EQUALS(0, vartok->valueType()->constness);
            ASSERT_EQUALS(0, vartok->valueType()->pointer);
            ASSERT_EQUALS(ValueType::SIGNED, vartok->valueType()->sign);
            ASSERT_EQUALS(ValueType::INT, vartok->valueType()->type);
        }

        // auto = const int *, v3 = const int * (const int[10])
        autotok = Token::findsimplematch(autotok, "auto v3");
        ASSERT(db && autotok && autotok->valueType());
        if (db && autotok && autotok->valueType()) {
            TODO_ASSERT_EQUALS(1, 0, autotok->valueType()->constness);
            ASSERT_EQUALS(1, autotok->valueType()->pointer);
            ASSERT_EQUALS(ValueType::SIGNED, autotok->valueType()->sign);
            ASSERT_EQUALS(ValueType::INT, autotok->valueType()->type);
        }
        vartok = Token::findsimplematch(autotok, "v3 =");
        ASSERT(db && vartok && vartok->valueType());
        if (db && vartok && vartok->valueType()) {
            TODO_ASSERT_EQUALS(1, 0, vartok->valueType()->constness);
            ASSERT_EQUALS(1, vartok->valueType()->pointer);
            ASSERT_EQUALS(ValueType::SIGNED, vartok->valueType()->sign);
            ASSERT_EQUALS(ValueType::INT, vartok->valueType()->type);
        }

        // auto = int *, v4 = const int * (const int[10])
        autotok = Token::findsimplematch(autotok, "auto v4");
        ASSERT(db && autotok && autotok->valueType());
        if (db && autotok && autotok->valueType()) {
            TODO_ASSERT_EQUALS(0, 1, autotok->valueType()->constness);
            ASSERT_EQUALS(1, autotok->valueType()->pointer);
            ASSERT_EQUALS(ValueType::SIGNED, autotok->valueType()->sign);
            ASSERT_EQUALS(ValueType::INT, autotok->valueType()->type);
        }
        vartok = Token::findsimplematch(autotok, "v4 =");
        ASSERT(db && vartok && vartok->valueType());
        if (db && vartok && vartok->valueType()) {
            TODO_ASSERT_EQUALS(1, 0, vartok->valueType()->constness);
            ASSERT_EQUALS(1, vartok->valueType()->pointer);
            ASSERT_EQUALS(ValueType::SIGNED, vartok->valueType()->sign);
            ASSERT_EQUALS(ValueType::INT, vartok->valueType()->type);
        }

        // auto = int, v5 = const int * (const int[10])
        autotok = Token::findsimplematch(autotok, "auto * v5");
        ASSERT(db && autotok && autotok->valueType());
        if (db && autotok && autotok->valueType()) {
            TODO_ASSERT_EQUALS(0, 1, autotok->valueType()->constness);
            ASSERT_EQUALS(0, autotok->valueType()->pointer);
            ASSERT_EQUALS(ValueType::SIGNED, autotok->valueType()->sign);
            ASSERT_EQUALS(ValueType::INT, autotok->valueType()->type);
        }
        vartok = Token::findsimplematch(autotok, "v5 =");
        ASSERT(db && vartok && vartok->valueType());
        if (db && vartok && vartok->valueType()) {
            TODO_ASSERT_EQUALS(1, 0, vartok->valueType()->constness);
            ASSERT_EQUALS(1, vartok->valueType()->pointer);
            ASSERT_EQUALS(ValueType::SIGNED, vartok->valueType()->sign);
            ASSERT_EQUALS(ValueType::INT, vartok->valueType()->type);
        }

        // auto = int, v6 = int
        autotok = Token::findsimplematch(autotok, "auto v6");
        ASSERT(db && autotok && autotok->valueType());
        if (db && autotok && autotok->valueType()) {
            ASSERT_EQUALS(0, autotok->valueType()->constness);
            ASSERT_EQUALS(0, autotok->valueType()->pointer);
            ASSERT_EQUALS(ValueType::SIGNED, autotok->valueType()->sign);
            ASSERT_EQUALS(ValueType::INT, autotok->valueType()->type);
        }
        vartok = Token::findsimplematch(autotok, "v6 =");
        ASSERT(db && vartok && vartok->valueType());
        if (db && vartok && vartok->valueType()) {
            ASSERT_EQUALS(0, vartok->valueType()->constness);
            ASSERT_EQUALS(0, vartok->valueType()->pointer);
            ASSERT_EQUALS(ValueType::SIGNED, vartok->valueType()->sign);
            ASSERT_EQUALS(ValueType::INT, vartok->valueType()->type);
        }

        // auto = int, v7 = int
        autotok = Token::findsimplematch(autotok, "auto v7");
        ASSERT(db && autotok && autotok->valueType());
        if (db && autotok && autotok->valueType()) {
            ASSERT_EQUALS(0, autotok->valueType()->constness);
            ASSERT_EQUALS(0, autotok->valueType()->pointer);
            ASSERT_EQUALS(ValueType::SIGNED, autotok->valueType()->sign);
            ASSERT_EQUALS(ValueType::INT, autotok->valueType()->type);
        }
        vartok = Token::findsimplematch(autotok, "v7 =");
        ASSERT(db && vartok && vartok->valueType());
        if (db && vartok && vartok->valueType()) {
            ASSERT_EQUALS(0, vartok->valueType()->constness);
            ASSERT_EQUALS(0, vartok->valueType()->pointer);
            ASSERT_EQUALS(ValueType::SIGNED, vartok->valueType()->sign);
            ASSERT_EQUALS(ValueType::INT, vartok->valueType()->type);
        }

        // auto = const int *, v8 = const int * (const int[10])
        autotok = Token::findsimplematch(autotok, "auto v8");
        ASSERT(db && autotok && autotok->valueType());
        if (db && autotok && autotok->valueType()) {
            TODO_ASSERT_EQUALS(1, 0, autotok->valueType()->constness);
            ASSERT_EQUALS(1, autotok->valueType()->pointer);
            ASSERT_EQUALS(ValueType::SIGNED, autotok->valueType()->sign);
            ASSERT_EQUALS(ValueType::INT, autotok->valueType()->type);
        }
        vartok = Token::findsimplematch(autotok, "v8 =");
        ASSERT(db && vartok && vartok->valueType());
        if (db && vartok && vartok->valueType()) {
            TODO_ASSERT_EQUALS(1, 0, vartok->valueType()->constness);
            ASSERT_EQUALS(1, vartok->valueType()->pointer);
            ASSERT_EQUALS(ValueType::SIGNED, vartok->valueType()->sign);
            ASSERT_EQUALS(ValueType::INT, vartok->valueType()->type);
        }

        // auto = int *, v9 = const int * (const int[10])
        autotok = Token::findsimplematch(autotok, "auto v9");
        ASSERT(db && autotok && autotok->valueType());
        if (db && autotok && autotok->valueType()) {
            TODO_ASSERT_EQUALS(0, 1, autotok->valueType()->constness);
            ASSERT_EQUALS(1, autotok->valueType()->pointer);
            ASSERT_EQUALS(ValueType::SIGNED, autotok->valueType()->sign);
            ASSERT_EQUALS(ValueType::INT, autotok->valueType()->type);
        }
        vartok = Token::findsimplematch(autotok, "v9 =");
        ASSERT(db && vartok && vartok->valueType());
        if (db && vartok && vartok->valueType()) {
            TODO_ASSERT_EQUALS(1, 0, vartok->valueType()->constness);
            ASSERT_EQUALS(1, vartok->valueType()->pointer);
            ASSERT_EQUALS(ValueType::SIGNED, vartok->valueType()->sign);
            ASSERT_EQUALS(ValueType::INT, vartok->valueType()->type);
        }

        // auto = int, v10 = const int * (const int[10])
        autotok = Token::findsimplematch(autotok, "auto * v10");
        ASSERT(db && autotok && autotok->valueType());
        if (db && autotok && autotok->valueType()) {
            TODO_ASSERT_EQUALS(0, 1, autotok->valueType()->constness);
            ASSERT_EQUALS(0, autotok->valueType()->pointer);
            ASSERT_EQUALS(ValueType::SIGNED, autotok->valueType()->sign);
            ASSERT_EQUALS(ValueType::INT, autotok->valueType()->type);
        }
        vartok = Token::findsimplematch(autotok, "v10 =");
        ASSERT(db && vartok && vartok->valueType());
        if (db && vartok && vartok->valueType()) {
            TODO_ASSERT_EQUALS(1, 0, vartok->valueType()->constness);
            ASSERT_EQUALS(1, vartok->valueType()->pointer);
            ASSERT_EQUALS(ValueType::SIGNED, vartok->valueType()->sign);
            ASSERT_EQUALS(ValueType::INT, vartok->valueType()->type);
        }

        // auto = int, v11 = int
        autotok = Token::findsimplematch(autotok, "auto v11");
        TODO_ASSERT(db && autotok && autotok->valueType());
        if (db && autotok && autotok->valueType()) {
            ASSERT_EQUALS(0, autotok->valueType()->constness);
            ASSERT_EQUALS(0, autotok->valueType()->pointer);
            ASSERT_EQUALS(ValueType::SIGNED, autotok->valueType()->sign);
            ASSERT_EQUALS(ValueType::INT, autotok->valueType()->type);
        }
        vartok = autotok->next();
        TODO_ASSERT(db && vartok && vartok->valueType());
        if (db && vartok && vartok->valueType()) {
            ASSERT_EQUALS(0, vartok->valueType()->constness);
            ASSERT_EQUALS(0, vartok->valueType()->pointer);
            ASSERT_EQUALS(ValueType::SIGNED, vartok->valueType()->sign);
            ASSERT_EQUALS(ValueType::INT, vartok->valueType()->type);
        }
    }

    void auto8() {
        GET_SYMBOL_DB("std::vector<int> vec;\n"
                      "void foo() {\n"
                      "    for (auto it = vec.begin(); it != vec.end(); ++it) { }\n"
                      "}");
        const Token *autotok = Token::findsimplematch(tokenizer.tokens(), "auto it");

        ASSERT(db && autotok && autotok->valueType());
        if (db && autotok && autotok->valueType()) {
            ASSERT_EQUALS(0, autotok->valueType()->constness);
            ASSERT_EQUALS(0, autotok->valueType()->pointer);
            ASSERT_EQUALS(ValueType::UNKNOWN_SIGN, autotok->valueType()->sign);
            ASSERT_EQUALS(ValueType::ITERATOR, autotok->valueType()->type);
        }

        vartok = Token::findsimplematch(autotok, "it =");
        ASSERT(db && vartok && vartok->valueType());
        if (db && vartok && vartok->valueType()) {
            ASSERT_EQUALS(0, vartok->valueType()->constness);
            ASSERT_EQUALS(0, vartok->valueType()->pointer);
            ASSERT_EQUALS(ValueType::UNKNOWN_SIGN, vartok->valueType()->sign);
            ASSERT_EQUALS(ValueType::ITERATOR, vartok->valueType()->type);
        }
    }

    void auto9() { // #8044 (segmentation fault)
        GET_SYMBOL_DB("class DHTTokenTracker {\n"
                      "  static const size_t SECRET_SIZE = 4;\n"
                      "  unsigned char secret_[2][SECRET_SIZE];\n"
                      "  void validateToken();\n"
                      "};\n"
                      "template <typename T> struct DerefEqual<T> derefEqual(const T& t) {\n"
                      "  return DerefEqual<T>(t);\n"
                      "}\n"
                      "template <typename T>\n"
                      "struct RefLess {\n"
                      "  bool operator()(const std::shared_ptr<T>& lhs,\n"
                      "                  const std::shared_ptr<T>& rhs)\n"
                      "  {\n"
                      "    return lhs.get() < rhs.get();\n"
                      "  }\n"
                      "};\n"
                      "void DHTTokenTracker::validateToken()\n"
                      "{\n"
                      "  for (auto& elem : secret_) {\n"
                      "  }\n"
                      "}");
        ASSERT_EQUALS(true,  db != nullptr); // not null
    }

    void auto10() { // #8020
        GET_SYMBOL_DB("void f() {\n"
                      "    std::vector<int> ints(4);\n"
                      "    auto iter = ints.begin() + (ints.size() - 1);\n"
                      "}");
        const Token *autotok = Token::findsimplematch(tokenizer.tokens(), "auto iter");

        ASSERT(db && autotok && autotok->valueType());
        if (db && autotok && autotok->valueType()) {
            ASSERT_EQUALS(0, autotok->valueType()->constness);
            ASSERT_EQUALS(0, autotok->valueType()->pointer);
            ASSERT_EQUALS(ValueType::UNKNOWN_SIGN, autotok->valueType()->sign);
            ASSERT_EQUALS(ValueType::ITERATOR, autotok->valueType()->type);
        }
    }

    void unionWithConstructor() {
        GET_SYMBOL_DB("union Fred {\n"
                      "    Fred(int x) : i(x) { }\n"
                      "    Fred(float x) : f(x) { }\n"
                      "    int i;\n"
                      "    float f;\n"
                      "};");

        ASSERT_EQUALS("", errout.str());

        const Token *f = Token::findsimplematch(tokenizer.tokens(), "Fred ( int");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 2);

        f = Token::findsimplematch(tokenizer.tokens(), "Fred ( float");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 3);
    }

    void using1() {
        const Standards::cppstd_t original_std = settings1.standards.cpp;
        settings1.standards.cpp = Standards::CPP11;
        GET_SYMBOL_DB("using INT = int;\n"
                      "using PINT = INT *;\n"
                      "using PCINT = const PINT;\n"
                      "INT i;\n"
                      "PINT pi;\n"
                      "PCINT pci;");
        settings1.standards.cpp = original_std;
        const Token *tok = Token::findsimplematch(tokenizer.tokens(), "INT i ;");

        ASSERT(db && tok && tok->next() && tok->next()->valueType());
        if (db && tok && tok->next() && tok->next()->valueType()) {
            tok = tok->next();
            ASSERT_EQUALS(0, tok->valueType()->constness);
            ASSERT_EQUALS(0, tok->valueType()->pointer);
            ASSERT_EQUALS(ValueType::SIGNED, tok->valueType()->sign);
            ASSERT_EQUALS(ValueType::INT, tok->valueType()->type);
        }

        tok = Token::findsimplematch(tokenizer.tokens(), "PINT pi ;");

        ASSERT(db && tok && tok->next() && tok->next()->valueType());
        if (db && tok && tok->next() && tok->next()->valueType()) {
            tok = tok->next();
            ASSERT_EQUALS(0, tok->valueType()->constness);
            ASSERT_EQUALS(1, tok->valueType()->pointer);
            ASSERT_EQUALS(ValueType::SIGNED, tok->valueType()->sign);
            ASSERT_EQUALS(ValueType::INT, tok->valueType()->type);
        }

        tok = Token::findsimplematch(tokenizer.tokens(), "PCINT pci ;");

        ASSERT(db && tok && tok->next() && tok->next()->valueType());
        if (db && tok && tok->next() && tok->next()->valueType()) {
            tok = tok->next();
            ASSERT_EQUALS(1, tok->valueType()->constness);
            ASSERT_EQUALS(1, tok->valueType()->pointer);
            ASSERT_EQUALS(ValueType::SIGNED, tok->valueType()->sign);
            ASSERT_EQUALS(ValueType::INT, tok->valueType()->type);
        }
    }

    void using2() { // #8331 (segmentation fault)
        const Standards::cppstd_t original_std = settings1.standards.cpp;
        settings1.standards.cpp = Standards::CPP11;

        {
            GET_SYMBOL_DB("using pboolean = pboolean;\n"
                          "pboolean b;");
            const Token *tok = Token::findsimplematch(tokenizer.tokens(), "b ;");

            ASSERT(db && tok && !tok->valueType());
        }

        {
            GET_SYMBOL_DB("using pboolean = bool;\n"
                          "using pboolean = pboolean;\n"
                          "pboolean b;");
            const Token *tok = Token::findsimplematch(tokenizer.tokens(), "b ;");

            ASSERT(db && tok && tok->valueType());
            if (db && tok && tok->valueType()) {
                ASSERT_EQUALS(0, tok->valueType()->constness);
                ASSERT_EQUALS(0, tok->valueType()->pointer);
                ASSERT_EQUALS(ValueType::UNKNOWN_SIGN, tok->valueType()->sign);
                ASSERT_EQUALS(ValueType::BOOL, tok->valueType()->type);
            }
        }

        settings1.standards.cpp = original_std;
    }

    void using3() { // #8343 (segmentation fault)
        const Standards::cppstd_t original_std = settings1.standards.cpp;
        settings1.standards.cpp = Standards::CPP11;
        GET_SYMBOL_DB("template <typename T>\n"
                      "using vector = typename MemoryModel::template vector<T>;\n"
                      "vector<uninitialized_uint64> m_bits;");
        settings1.standards.cpp = original_std;

        ASSERT(db != nullptr);
        ASSERT_EQUALS("", errout.str());
    }
};

REGISTER_TEST(TestSymbolDatabase)
