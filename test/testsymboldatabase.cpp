/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2023 Cppcheck team.
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
#include "symboldatabase.h"
#include "errortypes.h"
#include "fixture.h"
#include "helpers.h"
#include "token.h"
#include "tokenize.h"
#include "tokenlist.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <limits>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

class TestSymbolDatabase;

#define GET_SYMBOL_DB_STD(code) \
    Tokenizer tokenizer(&settings1, this); \
    LOAD_LIB_2(settings1.library, "std.cfg"); \
    const SymbolDatabase *db = getSymbolDB_inner(tokenizer, code, "test.cpp"); \
    ASSERT(db); \
    do {} while (false)

#define GET_SYMBOL_DB(code) \
    Tokenizer tokenizer(&settings1, this); \
    const SymbolDatabase *db = getSymbolDB_inner(tokenizer, code, "test.cpp"); \
    ASSERT(db); \
    do {} while (false)

#define GET_SYMBOL_DB_C(code) \
    Tokenizer tokenizer(&settings1, this); \
    const SymbolDatabase *db = getSymbolDB_inner(tokenizer, code, "test.c"); \
    do {} while (false)

class TestSymbolDatabase : public TestFixture {
public:
    TestSymbolDatabase() : TestFixture("TestSymbolDatabase") {}

private:
    const Token* vartok{nullptr};
    const Token* typetok{nullptr};
    Settings settings1 = settingsBuilder().library("std.cfg").build();
    const Settings settings2 = settingsBuilder().platform(cppcheck::Platform::Type::Unspecified).build();

    void reset() {
        vartok = nullptr;
        typetok = nullptr;
    }

    const static SymbolDatabase* getSymbolDB_inner(Tokenizer& tokenizer, const char* code, const char* filename) {
        errout.str("");
        std::istringstream istr(code);
        return tokenizer.tokenize(istr, filename) ? tokenizer.getSymbolDatabase() : nullptr;
    }

    static const Scope *findFunctionScopeByToken(const SymbolDatabase * db, const Token *tok) {
        std::list<Scope>::const_iterator scope;

        for (scope = db->scopeList.cbegin(); scope != db->scopeList.cend(); ++scope) {
            if (scope->type == Scope::eFunction) {
                if (scope->classDef == tok)
                    return &(*scope);
            }
        }
        return nullptr;
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
            auto it = std::find_if(currScope->functionList.cbegin(), currScope->functionList.cend(), [&](const Function& f) {
                return f.tokenDef->str() == str;
            });
            if (it != currScope->functionList.end())
                return &*it;
            currScope = currScope->nestedIn;
        }
        return nullptr;
    }

    void run() override {
        TEST_CASE(array);
        TEST_CASE(array_ptr);
        TEST_CASE(stlarray1);
        TEST_CASE(stlarray2);
        TEST_CASE(stlarray3);

        TEST_CASE(test_isVariableDeclarationCanHandleNull);
        TEST_CASE(test_isVariableDeclarationIdentifiesSimpleDeclaration);
        TEST_CASE(test_isVariableDeclarationIdentifiesInitialization);
        TEST_CASE(test_isVariableDeclarationIdentifiesCpp11Initialization);
        TEST_CASE(test_isVariableDeclarationIdentifiesScopedDeclaration);
        TEST_CASE(test_isVariableDeclarationIdentifiesStdDeclaration);
        TEST_CASE(test_isVariableDeclarationIdentifiesScopedStdDeclaration);
        TEST_CASE(test_isVariableDeclarationIdentifiesManyScopes);
        TEST_CASE(test_isVariableDeclarationIdentifiesPointers);
        TEST_CASE(test_isVariableDeclarationIdentifiesPointers2);
        TEST_CASE(test_isVariableDeclarationDoesNotIdentifyConstness);
        TEST_CASE(test_isVariableDeclarationIdentifiesFirstOfManyVariables);
        TEST_CASE(test_isVariableDeclarationIdentifiesScopedPointerDeclaration);
        TEST_CASE(test_isVariableDeclarationIdentifiesDeclarationWithIndirection);
        TEST_CASE(test_isVariableDeclarationIdentifiesDeclarationWithMultipleIndirection);
        TEST_CASE(test_isVariableDeclarationIdentifiesArray);
        TEST_CASE(test_isVariableDeclarationIdentifiesPointerArray);
        TEST_CASE(test_isVariableDeclarationIdentifiesOfArrayPointers);
        TEST_CASE(test_isVariableDeclarationIdentifiesArrayOfFunctionPointers);
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
        TEST_CASE(isVariableDeclarationIf);
        TEST_CASE(isVariableStlType);
        TEST_CASE(isVariablePointerToConstPointer);
        TEST_CASE(isVariablePointerToVolatilePointer);
        TEST_CASE(isVariablePointerToConstVolatilePointer);
        TEST_CASE(isVariableMultiplePointersAndQualifiers);
        TEST_CASE(variableVolatile);
        TEST_CASE(variableConstexpr);
        TEST_CASE(isVariableDecltype);
        TEST_CASE(isVariableAlignas);

        TEST_CASE(VariableValueType1);
        TEST_CASE(VariableValueType2);
        TEST_CASE(VariableValueType3);
        TEST_CASE(VariableValueType4); // smart pointer type
        TEST_CASE(VariableValueType5); // smart pointer type
        TEST_CASE(VariableValueTypeReferences);

        TEST_CASE(findVariableType1);
        TEST_CASE(findVariableType2);
        TEST_CASE(findVariableType3);
        TEST_CASE(findVariableTypeExternC);

        TEST_CASE(rangeBasedFor);

        TEST_CASE(memberVar1);
        TEST_CASE(arrayMemberVar1);
        TEST_CASE(arrayMemberVar2);
        TEST_CASE(arrayMemberVar3);
        TEST_CASE(arrayMemberVar4);
        TEST_CASE(staticMemberVar);
        TEST_CASE(getVariableFromVarIdBoundsCheck);

        TEST_CASE(hasRegularFunction);
        TEST_CASE(hasRegularFunction_trailingReturnType);
        TEST_CASE(hasInlineClassFunction);
        TEST_CASE(hasInlineClassFunction_trailingReturnType);
        TEST_CASE(hasMissingInlineClassFunction);
        TEST_CASE(hasClassFunction);
        TEST_CASE(hasClassFunction_trailingReturnType);
        TEST_CASE(hasClassFunction_decltype_auto);

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
        TEST_CASE(functionDeclarations2);
        TEST_CASE(constexprFunction);
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
        TEST_CASE(functionArgs14); // #9055
        TEST_CASE(functionArgs15); // #7159
        TEST_CASE(functionArgs16); // #9591
        TEST_CASE(functionArgs17);
        TEST_CASE(functionArgs18); // #10376
        TEST_CASE(functionArgs19); // #10376
        TEST_CASE(functionArgs20);

        TEST_CASE(functionImplicitlyVirtual);
        TEST_CASE(functionGetOverridden);

        TEST_CASE(functionIsInlineKeyword);

        TEST_CASE(functionStatic);

        TEST_CASE(functionReturnsReference); // Function::returnsReference

        TEST_CASE(namespaces1);
        TEST_CASE(namespaces2);
        TEST_CASE(namespaces3);  // #3854 - unknown macro
        TEST_CASE(namespaces4);
        TEST_CASE(needInitialization);

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
        TEST_CASE(symboldatabase74); // #8838 - final
        TEST_CASE(symboldatabase75);
        TEST_CASE(symboldatabase76); // #9056
        TEST_CASE(symboldatabase77); // #8663
        TEST_CASE(symboldatabase78); // #9147
        TEST_CASE(symboldatabase79); // #9392
        TEST_CASE(symboldatabase80); // #9389
        TEST_CASE(symboldatabase81); // #9411
        TEST_CASE(symboldatabase82);
        TEST_CASE(symboldatabase83); // #9431
        TEST_CASE(symboldatabase84);
        TEST_CASE(symboldatabase85);
        TEST_CASE(symboldatabase86);
        TEST_CASE(symboldatabase87); // #9922 'extern const char ( * x [ 256 ] ) ;'
        TEST_CASE(symboldatabase88); // #10040 (using namespace)
        TEST_CASE(symboldatabase89); // valuetype name
        TEST_CASE(symboldatabase90);
        TEST_CASE(symboldatabase91);
        TEST_CASE(symboldatabase92); // daca crash
        TEST_CASE(symboldatabase93); // alignas attribute
        TEST_CASE(symboldatabase94); // structured bindings
        TEST_CASE(symboldatabase95); // #10295
        TEST_CASE(symboldatabase96); // #10126
        TEST_CASE(symboldatabase97); // #10598 - final class
        TEST_CASE(symboldatabase98); // #10451
        TEST_CASE(symboldatabase99); // #10864
        TEST_CASE(symboldatabase100); // #10174
        TEST_CASE(symboldatabase101);
        TEST_CASE(symboldatabase102);
        TEST_CASE(symboldatabase103);
        TEST_CASE(symboldatabase104);

        TEST_CASE(createSymbolDatabaseFindAllScopes1);
        TEST_CASE(createSymbolDatabaseFindAllScopes2);
        TEST_CASE(createSymbolDatabaseFindAllScopes3);
        TEST_CASE(createSymbolDatabaseFindAllScopes4);
        TEST_CASE(createSymbolDatabaseFindAllScopes5);
        TEST_CASE(createSymbolDatabaseFindAllScopes6);

        TEST_CASE(enum1);
        TEST_CASE(enum2);
        TEST_CASE(enum3);
        TEST_CASE(enum4);
        TEST_CASE(enum5);
        TEST_CASE(enum6);
        TEST_CASE(enum7);
        TEST_CASE(enum8);
        TEST_CASE(enum9);
        TEST_CASE(enum10); // #11001
        TEST_CASE(enum11);
        TEST_CASE(enum12);
        TEST_CASE(enum13);

        TEST_CASE(sizeOfType);

        TEST_CASE(isImplicitlyVirtual);
        TEST_CASE(isPure);

        TEST_CASE(isFunction1); // UNKNOWN_MACRO(a,b) { .. }
        TEST_CASE(isFunction2);
        TEST_CASE(isFunction3);

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
        TEST_CASE(findFunction12);
        TEST_CASE(findFunction13);
        TEST_CASE(findFunction14);
        TEST_CASE(findFunction15);
        TEST_CASE(findFunction16);
        TEST_CASE(findFunction17);
        TEST_CASE(findFunction18);
        TEST_CASE(findFunction19);
        TEST_CASE(findFunction20); // #8280
        TEST_CASE(findFunction21);
        TEST_CASE(findFunction22);
        TEST_CASE(findFunction23);
        TEST_CASE(findFunction24); // smart pointer
        TEST_CASE(findFunction25); // std::vector<std::shared_ptr<Fred>>
        TEST_CASE(findFunction26); // #8668 - pointer parameter in function call, const pointer function argument
        TEST_CASE(findFunction27);
        TEST_CASE(findFunction28);
        TEST_CASE(findFunction29);
        TEST_CASE(findFunction30);
        TEST_CASE(findFunction31);
        TEST_CASE(findFunction32); // C: relax type matching
        TEST_CASE(findFunction33); // #9885 variadic function
        TEST_CASE(findFunction34); // #10061
        TEST_CASE(findFunction35);
        TEST_CASE(findFunction36); // #10122
        TEST_CASE(findFunction37); // #10124
        TEST_CASE(findFunction38); // #10125
        TEST_CASE(findFunction39); // #10127
        TEST_CASE(findFunction40); // #10135
        TEST_CASE(findFunction41); // #10202
        TEST_CASE(findFunction42);
        TEST_CASE(findFunction43); // #10087
        TEST_CASE(findFunction44); // #11182
        TEST_CASE(findFunction45);
        TEST_CASE(findFunction46);
        TEST_CASE(findFunction47);
        TEST_CASE(findFunction48);
        TEST_CASE(findFunction49); // #11888
        TEST_CASE(findFunction50); // #11904 - method with same name and arguments in derived class
        TEST_CASE(findFunctionContainer);
        TEST_CASE(findFunctionExternC);
        TEST_CASE(findFunctionGlobalScope); // ::foo

        TEST_CASE(overloadedFunction1);

        TEST_CASE(valueTypeMatchParameter); // ValueType::matchParameter

        TEST_CASE(noexceptFunction1);
        TEST_CASE(noexceptFunction2);
        TEST_CASE(noexceptFunction3);
        TEST_CASE(noexceptFunction4);

        TEST_CASE(throwFunction1);
        TEST_CASE(throwFunction2);

        TEST_CASE(nothrowAttributeFunction);
        TEST_CASE(nothrowDeclspecFunction);

        TEST_CASE(noreturnAttributeFunction);
        TEST_CASE(nodiscardAttributeFunction);

        TEST_CASE(varTypesIntegral); // known integral
        TEST_CASE(varTypesFloating); // known floating
        TEST_CASE(varTypesOther);    // (un)known

        TEST_CASE(functionPrototype); // #5867

        TEST_CASE(lambda); // #5867
        TEST_CASE(lambda2); // #7473
        TEST_CASE(lambda3);
        TEST_CASE(lambda4);
        TEST_CASE(lambda5);

        TEST_CASE(circularDependencies); // #6298

        TEST_CASE(executableScopeWithUnknownFunction);

        TEST_CASE(valueType1);
        TEST_CASE(valueType2);
        TEST_CASE(valueType3);
        TEST_CASE(valueTypeThis);

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
        TEST_CASE(auto11); // #8964 - const auto startX = x;
        TEST_CASE(auto12); // #8993 - const std::string &x; auto y = x; if (y.empty()) ..
        TEST_CASE(auto13);
        TEST_CASE(auto14);
        TEST_CASE(auto15); // C++17 auto deduction from braced-init-list
        TEST_CASE(auto16);
        TEST_CASE(auto17); // #11163
        TEST_CASE(auto18);
        TEST_CASE(auto19);
        TEST_CASE(auto20);
        TEST_CASE(auto21);
        TEST_CASE(auto22);

        TEST_CASE(unionWithConstructor);

        TEST_CASE(incomplete_type); // #9255 (infinite recursion)
    }

    void array() {
        GET_SYMBOL_DB_C("int a[10+2];");
        ASSERT(db != nullptr);

        ASSERT(db->variableList().size() == 2); // the first one is not used
        const Variable * v = db->getVariableFromVarId(1);
        ASSERT(v != nullptr);

        ASSERT(v->isArray());
        ASSERT_EQUALS(1U, v->dimensions().size());
        ASSERT_EQUALS(12U, v->dimension(0));
    }

    void array_ptr() {
        GET_SYMBOL_DB("const char* a[] = { \"abc\" };\n"
                      "const char* b[] = { \"def\", \"ghijkl\" };");
        ASSERT(db != nullptr);

        ASSERT(db->variableList().size() == 3); // the first one is not used
        const Variable* v = db->getVariableFromVarId(1);
        ASSERT(v != nullptr);

        ASSERT(v->isArray());
        ASSERT(v->isPointerArray());
        ASSERT_EQUALS(1U, v->dimensions().size());
        ASSERT_EQUALS(1U, v->dimension(0));

        v = db->getVariableFromVarId(2);
        ASSERT(v != nullptr);

        ASSERT(v->isArray());
        ASSERT(v->isPointerArray());
        ASSERT_EQUALS(1U, v->dimensions().size());
        ASSERT_EQUALS(2U, v->dimension(0));
    }

    void stlarray1() {
        GET_SYMBOL_DB("std::array<int, 16 + 4> arr;");
        ASSERT(db != nullptr);

        ASSERT_EQUALS(2, db->variableList().size()); // the first one is not used
        const Variable * v = db->getVariableFromVarId(1);
        ASSERT(v != nullptr);

        ASSERT(v->isArray());
        ASSERT_EQUALS(1U, v->dimensions().size());
        ASSERT_EQUALS(20U, v->dimension(0));
    }

    void stlarray2() {
        GET_SYMBOL_DB("constexpr int sz = 16; std::array<int, sz + 4> arr;");
        ASSERT(db != nullptr);

        ASSERT_EQUALS(3, db->variableList().size()); // the first one is not used
        const Variable * v = db->getVariableFromVarId(2);
        ASSERT(v != nullptr);

        ASSERT(v->isArray());
        ASSERT_EQUALS(1U, v->dimensions().size());
        ASSERT_EQUALS(20U, v->dimension(0));
    }

    void stlarray3() {
        GET_SYMBOL_DB("std::array<int, 4> a;\n"
                      "std::array<int, 4> b[2];\n"
                      "const std::array<int, 4>& r = a;\n");
        ASSERT(db != nullptr);

        ASSERT_EQUALS(4, db->variableList().size()); // the first one is not used
        auto it = db->variableList().begin() + 1;

        ASSERT((*it)->isArray());
        ASSERT(!(*it)->isPointer());
        ASSERT(!(*it)->isReference());
        ASSERT_EQUALS(1U, (*it)->dimensions().size());
        ASSERT_EQUALS(4U, (*it)->dimension(0));
        const ValueType* vt = (*it)->valueType();
        ASSERT(vt && vt->container);
        ASSERT_EQUALS(vt->pointer, 0);
        const Token* tok = (*it)->nameToken();
        ASSERT(tok && (vt = tok->valueType()));
        ASSERT_EQUALS(vt->pointer, 0);

        ++it;
        ASSERT((*it)->isArray());
        ASSERT(!(*it)->isPointer());
        ASSERT(!(*it)->isReference());
        ASSERT_EQUALS(1U, (*it)->dimensions().size());
        ASSERT_EQUALS(4U, (*it)->dimension(0));
        vt = (*it)->valueType();
        ASSERT_EQUALS(vt->pointer, 0);
        tok = (*it)->nameToken();
        ASSERT(tok && (vt = tok->valueType()));
        ASSERT_EQUALS(vt->pointer, 1);

        ++it;
        ASSERT((*it)->isArray());
        ASSERT(!(*it)->isPointer());
        ASSERT((*it)->isReference());
        ASSERT((*it)->isConst());
        ASSERT_EQUALS(1U, (*it)->dimensions().size());
        ASSERT_EQUALS(4U, (*it)->dimension(0));
        vt = (*it)->valueType();
        ASSERT_EQUALS(vt->pointer, 0);
        ASSERT(vt->reference == Reference::LValue);
        tok = (*it)->nameToken();
        ASSERT(tok && (vt = tok->valueType()));
        ASSERT_EQUALS(vt->pointer, 0);
        ASSERT_EQUALS(vt->constness, 1);
        ASSERT(vt->reference == Reference::LValue);
    }

    void test_isVariableDeclarationCanHandleNull() {
        reset();
        GET_SYMBOL_DB("void main(){}");
        const bool result = db->scopeList.front().isVariableDeclaration(nullptr, vartok, typetok);
        ASSERT_EQUALS(false, result);
        ASSERT(nullptr == vartok);
        ASSERT(nullptr == typetok);
        Variable v(nullptr, nullptr, nullptr, 0, AccessControl::Public, nullptr, nullptr, &settings1);
    }

    void test_isVariableDeclarationIdentifiesSimpleDeclaration() {
        reset();
        GET_SYMBOL_DB("int x;");
        const bool result = db->scopeList.front().isVariableDeclaration(tokenizer.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("x", vartok->str());
        ASSERT_EQUALS("int", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, AccessControl::Public, nullptr, nullptr, &settings1);
        ASSERT(false == v.isArray());
        ASSERT(false == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void test_isVariableDeclarationIdentifiesInitialization() {
        reset();
        GET_SYMBOL_DB("int x (1);");
        const bool result = db->scopeList.front().isVariableDeclaration(tokenizer.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("x", vartok->str());
        ASSERT_EQUALS("int", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, AccessControl::Public, nullptr, nullptr, &settings1);
        ASSERT(false == v.isArray());
        ASSERT(false == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void test_isVariableDeclarationIdentifiesCpp11Initialization() {
        reset();
        GET_SYMBOL_DB("int x {1};");
        const bool result = db->scopeList.front().isVariableDeclaration(tokenizer.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("x", vartok->str());
        ASSERT_EQUALS("int", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, AccessControl::Public, nullptr, nullptr, &settings1);
        ASSERT(false == v.isArray());
        ASSERT(false == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void test_isVariableDeclarationIdentifiesScopedDeclaration() {
        reset();
        GET_SYMBOL_DB("::int x;");
        const bool result = db->scopeList.front().isVariableDeclaration(tokenizer.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("x", vartok->str());
        ASSERT_EQUALS("int", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, AccessControl::Public, nullptr, nullptr, &settings1);
        ASSERT(false == v.isArray());
        ASSERT(false == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void test_isVariableDeclarationIdentifiesStdDeclaration() {
        reset();
        GET_SYMBOL_DB("std::string x;");
        const bool result = db->scopeList.front().isVariableDeclaration(tokenizer.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("x", vartok->str());
        ASSERT_EQUALS("string", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, AccessControl::Public, nullptr, nullptr, &settings1);
        ASSERT(false == v.isArray());
        ASSERT(false == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void test_isVariableDeclarationIdentifiesScopedStdDeclaration() {
        reset();
        GET_SYMBOL_DB("::std::string x;");
        const bool result = db->scopeList.front().isVariableDeclaration(tokenizer.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("x", vartok->str());
        ASSERT_EQUALS("string", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, AccessControl::Public, nullptr, nullptr, &settings1);
        ASSERT(false == v.isArray());
        ASSERT(false == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void test_isVariableDeclarationIdentifiesManyScopes() {
        reset();
        GET_SYMBOL_DB("AA::BB::CC::DD::EE x;");
        const bool result = db->scopeList.front().isVariableDeclaration(tokenizer.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("x", vartok->str());
        ASSERT_EQUALS("EE", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, AccessControl::Public, nullptr, nullptr, &settings1);
        ASSERT(false == v.isArray());
        ASSERT(false == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void test_isVariableDeclarationIdentifiesPointers() {
        {
            reset();
            GET_SYMBOL_DB("int* p;");
            const bool result1 = db->scopeList.front().isVariableDeclaration(tokenizer.tokens(), vartok, typetok);
            ASSERT_EQUALS(true, result1);
            ASSERT_EQUALS("p", vartok->str());
            ASSERT_EQUALS("int", typetok->str());
            Variable v1(vartok, typetok, vartok->previous(), 0, AccessControl::Public, nullptr, nullptr, &settings1);
            ASSERT(false == v1.isArray());
            ASSERT(true == v1.isPointer());
            ASSERT(false == v1.isReference());
        }
        {
            reset();
            givenACodeSampleToTokenize constpointer("const int* p;");
            Variable v2(constpointer.tokens()->tokAt(3), constpointer.tokens()->next(), constpointer.tokens()->tokAt(2), 0, AccessControl::Public, nullptr, nullptr, &settings1);
            ASSERT(false == v2.isArray());
            ASSERT(true == v2.isPointer());
            ASSERT(false == v2.isConst());
            ASSERT(false == v2.isReference());
        }
        {
            reset();
            GET_SYMBOL_DB("int* const p;");
            const bool result2 = db->scopeList.front().isVariableDeclaration(tokenizer.tokens(), vartok, typetok);
            ASSERT_EQUALS(true, result2);
            ASSERT_EQUALS("p", vartok->str());
            ASSERT_EQUALS("int", typetok->str());
            Variable v3(vartok, typetok, vartok->previous(), 0, AccessControl::Public, nullptr, nullptr, &settings1);
            ASSERT(false == v3.isArray());
            ASSERT(true == v3.isPointer());
            ASSERT(true == v3.isConst());
            ASSERT(false == v3.isReference());
        }
    }

    void test_isVariableDeclarationIdentifiesPointers2() {

        GET_SYMBOL_DB("void slurpInManifest() {\n"
                      "  std::string tmpiostring(*tI);\n"
                      "  if(tmpiostring==\"infoonly\"){}\n"
                      "}");

        const Token *tok = Token::findsimplematch(tokenizer.tokens(), "tmpiostring ==");
        ASSERT(tok->variable());
        ASSERT(!tok->variable()->isPointer());
    }

    void test_isVariableDeclarationDoesNotIdentifyConstness() {
        reset();
        GET_SYMBOL_DB("const int* cp;");
        const bool result = db->scopeList.front().isVariableDeclaration(tokenizer.tokens(), vartok, typetok);
        ASSERT_EQUALS(false, result);
        ASSERT(nullptr == vartok);
        ASSERT(nullptr == typetok);
    }

    void test_isVariableDeclarationIdentifiesFirstOfManyVariables() {
        reset();
        GET_SYMBOL_DB("int first, second;");
        const bool result = db->scopeList.front().isVariableDeclaration(tokenizer.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("first", vartok->str());
        ASSERT_EQUALS("int", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, AccessControl::Public, nullptr, nullptr, &settings1);
        ASSERT(false == v.isArray());
        ASSERT(false == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void test_isVariableDeclarationIdentifiesScopedPointerDeclaration() {
        reset();
        GET_SYMBOL_DB("AA::BB::CC::DD::EE* p;");
        const bool result = db->scopeList.front().isVariableDeclaration(tokenizer.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("p", vartok->str());
        ASSERT_EQUALS("EE", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, AccessControl::Public, nullptr, nullptr, &settings1);
        ASSERT(false == v.isArray());
        ASSERT(true == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void test_isVariableDeclarationIdentifiesDeclarationWithIndirection() {
        reset();
        GET_SYMBOL_DB("int** pp;");
        const bool result = db->scopeList.front().isVariableDeclaration(tokenizer.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("pp", vartok->str());
        ASSERT_EQUALS("int", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, AccessControl::Public, nullptr, nullptr, &settings1);
        ASSERT(false == v.isArray());
        ASSERT(true == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void test_isVariableDeclarationIdentifiesDeclarationWithMultipleIndirection() {
        reset();
        GET_SYMBOL_DB("int***** p;");
        const bool result = db->scopeList.front().isVariableDeclaration(tokenizer.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("p", vartok->str());
        ASSERT_EQUALS("int", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, AccessControl::Public, nullptr, nullptr, &settings1);
        ASSERT(false == v.isArray());
        ASSERT(true == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void test_isVariableDeclarationIdentifiesArray() {
        reset();
        GET_SYMBOL_DB("::std::string v[3];");
        const bool result = db->scopeList.front().isVariableDeclaration(tokenizer.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("v", vartok->str());
        ASSERT_EQUALS("string", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, AccessControl::Public, nullptr, nullptr, &settings1);
        ASSERT(true == v.isArray());
        ASSERT(false == v.isPointer());
        ASSERT(false == v.isPointerArray());
        ASSERT(false == v.isReference());
    }

    void test_isVariableDeclarationIdentifiesPointerArray() {
        reset();
        GET_SYMBOL_DB("A *a[5];");
        const bool result = db->scopeList.front().isVariableDeclaration(tokenizer.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("a", vartok->str());
        ASSERT_EQUALS("A", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, AccessControl::Public, nullptr, nullptr, &settings1);
        ASSERT(false == v.isPointer());
        ASSERT(true == v.isArray());
        ASSERT(false == v.isPointerToArray());
        ASSERT(true == v.isPointerArray());
        ASSERT(false == v.isReference());
    }

    void test_isVariableDeclarationIdentifiesOfArrayPointers() {
        reset();
        GET_SYMBOL_DB("A (*a)[5];");
        const bool result = db->scopeList.front().isVariableDeclaration(tokenizer.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("a", vartok->str());
        ASSERT_EQUALS("A", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, AccessControl::Public, nullptr, nullptr, &settings1);
        ASSERT(true == v.isPointer());
        ASSERT(false == v.isArray());
        ASSERT(true == v.isPointerToArray());
        ASSERT(false == v.isPointerArray());
        ASSERT(false == v.isReference());
    }

    void test_isVariableDeclarationIdentifiesArrayOfFunctionPointers() {
        reset();
        GET_SYMBOL_DB("int (*a[])(int) = { g };"); // #11596
        const bool result = db->scopeList.front().isVariableDeclaration(tokenizer.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("a", vartok->str());
        ASSERT_EQUALS("int", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, AccessControl::Public, nullptr, nullptr, &settings1);
        ASSERT(false == v.isPointer());
        ASSERT(true == v.isArray());
        ASSERT(false == v.isPointerToArray());
        ASSERT(true == v.isPointerArray());
        ASSERT(false == v.isReference());
        ASSERT(true == v.isInit());
    }

    void isVariableDeclarationIdentifiesTemplatedPointerVariable() {
        reset();
        GET_SYMBOL_DB("std::set<char>* chars;");
        const bool result = db->scopeList.front().isVariableDeclaration(tokenizer.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("chars", vartok->str());
        ASSERT_EQUALS("set", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, AccessControl::Public, nullptr, nullptr, &settings1);
        ASSERT(false == v.isArray());
        ASSERT(true == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void isVariableDeclarationIdentifiesTemplatedPointerToPointerVariable() {
        reset();
        GET_SYMBOL_DB("std::deque<int>*** ints;");
        const bool result = db->scopeList.front().isVariableDeclaration(tokenizer.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("ints", vartok->str());
        ASSERT_EQUALS("deque", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, AccessControl::Public, nullptr, nullptr, &settings1);
        ASSERT(false == v.isArray());
        ASSERT(true == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void isVariableDeclarationIdentifiesTemplatedArrayVariable() {
        reset();
        GET_SYMBOL_DB("std::deque<int> ints[3];");
        const bool result = db->scopeList.front().isVariableDeclaration(tokenizer.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("ints", vartok->str());
        ASSERT_EQUALS("deque", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, AccessControl::Public, nullptr, nullptr, &settings1);
        ASSERT(true == v.isArray());
        ASSERT(false == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void isVariableDeclarationIdentifiesTemplatedVariable() {
        reset();
        GET_SYMBOL_DB("std::vector<int> ints;");
        const bool result = db->scopeList.front().isVariableDeclaration(tokenizer.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("ints", vartok->str());
        ASSERT_EQUALS("vector", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, AccessControl::Public, nullptr, nullptr, &settings1);
        ASSERT(false == v.isArray());
        ASSERT(false == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void isVariableDeclarationIdentifiesTemplatedVariableIterator() {
        reset();
        GET_SYMBOL_DB("std::list<int>::const_iterator floats;");
        const bool result = db->scopeList.front().isVariableDeclaration(tokenizer.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("floats", vartok->str());
        ASSERT_EQUALS("const_iterator", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, AccessControl::Public, nullptr, nullptr, &settings1);
        ASSERT(false == v.isArray());
        ASSERT(false == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void isVariableDeclarationIdentifiesNestedTemplateVariable() {
        reset();
        GET_SYMBOL_DB("std::deque<std::set<int> > intsets;");
        const bool result = db->scopeList.front().isVariableDeclaration(tokenizer.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("intsets", vartok->str());
        ASSERT_EQUALS("deque", typetok->str());
        Variable v(vartok, typetok, vartok->previous(), 0, AccessControl::Public, nullptr, nullptr, &settings1);
        ASSERT(false == v.isArray());
        ASSERT(false == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void isVariableDeclarationIdentifiesReference() {
        {
            reset();
            GET_SYMBOL_DB("int& foo;");
            const bool result1 = db->scopeList.front().isVariableDeclaration(tokenizer.tokens(), vartok, typetok);
            ASSERT_EQUALS(true, result1);
            Variable v1(vartok, typetok, vartok->previous(), 0, AccessControl::Public, nullptr, nullptr, &settings1);
            ASSERT(false == v1.isArray());
            ASSERT(false == v1.isPointer());
            ASSERT(true == v1.isReference());
        }
        {
            reset();
            GET_SYMBOL_DB("foo*& bar;");
            const bool result2 = db->scopeList.front().isVariableDeclaration(tokenizer.tokens(), vartok, typetok);
            ASSERT_EQUALS(true, result2);
            Variable v2(vartok, typetok, vartok->previous(), 0, AccessControl::Public, nullptr, nullptr, &settings1);
            ASSERT(false == v2.isArray());
            ASSERT(true == v2.isPointer());
            ASSERT(true == v2.isReference());
        }
        {
            reset();
            GET_SYMBOL_DB("std::vector<int>& foo;");
            const bool result3 = db->scopeList.front().isVariableDeclaration(tokenizer.tokens(), vartok, typetok);
            ASSERT_EQUALS(true, result3);
            Variable v3(vartok, typetok, vartok->previous(), 0, AccessControl::Public, nullptr, nullptr, &settings1);
            ASSERT(false == v3.isArray());
            ASSERT(false == v3.isPointer());
            ASSERT(true == v3.isReference());
        }
    }

    void isVariableDeclarationDoesNotIdentifyTemplateClass() {
        reset();
        GET_SYMBOL_DB("template <class T> class SomeClass{};");
        const bool result = db->scopeList.front().isVariableDeclaration(tokenizer.tokens(), vartok, typetok);
        ASSERT_EQUALS(false, result);
    }

    void isVariableDeclarationDoesNotIdentifyCppCast() {
        reset();
        GET_SYMBOL_DB("reinterpret_cast <char *> (code)[0] = 0;");
        const bool result = db->scopeList.front().isVariableDeclaration(tokenizer.tokens(), vartok, typetok);
        ASSERT_EQUALS(false, result);
    }

    void isVariableDeclarationPointerConst() {
        reset();
        GET_SYMBOL_DB("std::string const* s;");
        const bool result = db->scopeList.front().isVariableDeclaration(tokenizer.tokens()->next(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        Variable v(vartok, typetok, vartok->previous(), 0, AccessControl::Public, nullptr, nullptr, &settings1);
        ASSERT(false == v.isArray());
        ASSERT(true == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void isVariableDeclarationRValueRef() {
        reset();
        GET_SYMBOL_DB("int&& i;");
        const bool result = db->scopeList.front().isVariableDeclaration(tokenizer.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        Variable v(vartok, typetok, vartok->previous(), 0, AccessControl::Public, nullptr, nullptr, &settings1);
        ASSERT(false == v.isArray());
        ASSERT(false == v.isPointer());
        ASSERT(true == v.isReference());
        ASSERT(true == v.isRValueReference());
        ASSERT(tokenizer.tokens()->tokAt(2)->scope() != nullptr);
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

    void isVariableDeclarationIf() {
        GET_SYMBOL_DB("void foo() {\n"
                      "    for (auto& elem : items) {\n"
                      "        if (auto x = bar()) { int y = 3; }\n"
                      "    }\n"
                      "}");
        const Token *x = Token::findsimplematch(tokenizer.tokens(), "x");
        ASSERT(x);
        ASSERT(x->varId());
        ASSERT(x->variable());

        const Token *y = Token::findsimplematch(tokenizer.tokens(), "y");
        ASSERT(y);
        ASSERT(y->varId());
        ASSERT(y->variable());
    }

    void VariableValueType1() {
        GET_SYMBOL_DB("typedef uint8_t u8;\n"
                      "static u8 x;");
        const Variable* x = db->getVariableFromVarId(1);
        ASSERT_EQUALS("x", x->name());
        ASSERT(x->valueType()->isIntegral());
    }

    void VariableValueType2() {
        GET_SYMBOL_DB("using u8 = uint8_t;\n"
                      "static u8 x;");
        const Variable* x = db->getVariableFromVarId(1);
        ASSERT_EQUALS("x", x->name());
        ASSERT(x->valueType()->isIntegral());
    }

    void VariableValueType3() {
        // std::string::size_type
        {
            GET_SYMBOL_DB("void f(std::string::size_type x);");
            const Variable* const x = db->getVariableFromVarId(1);
            ASSERT_EQUALS("x", x->name());
            // TODO: Configure std::string::size_type somehow.
            TODO_ASSERT_EQUALS(ValueType::Type::LONGLONG, ValueType::Type::UNKNOWN_INT, x->valueType()->type);
            ASSERT_EQUALS(ValueType::Sign::UNSIGNED, x->valueType()->sign);
        }
        // std::wstring::size_type
        {
            GET_SYMBOL_DB("void f(std::wstring::size_type x);");
            const Variable* const x = db->getVariableFromVarId(1);
            ASSERT_EQUALS("x", x->name());
            // TODO: Configure std::wstring::size_type somehow.
            TODO_ASSERT_EQUALS(ValueType::Type::LONGLONG, ValueType::Type::UNKNOWN_INT, x->valueType()->type);
            ASSERT_EQUALS(ValueType::Sign::UNSIGNED, x->valueType()->sign);
        }
        // std::u16string::size_type
        {
            GET_SYMBOL_DB("void f(std::u16string::size_type x);");
            const Variable* const x = db->getVariableFromVarId(1);
            ASSERT_EQUALS("x", x->name());
            // TODO: Configure std::u16string::size_type somehow.
            TODO_ASSERT_EQUALS(ValueType::Type::LONGLONG, ValueType::Type::UNKNOWN_INT, x->valueType()->type);
            ASSERT_EQUALS(ValueType::Sign::UNSIGNED, x->valueType()->sign);
        }
        // std::u32string::size_type
        {
            GET_SYMBOL_DB("void f(std::u32string::size_type x);");
            const Variable* const x = db->getVariableFromVarId(1);
            ASSERT_EQUALS("x", x->name());
            // TODO: Configure std::u32string::size_type somehow.
            TODO_ASSERT_EQUALS(ValueType::Type::LONGLONG, ValueType::Type::UNKNOWN_INT, x->valueType()->type);
            ASSERT_EQUALS(ValueType::Sign::UNSIGNED, x->valueType()->sign);
        }
    }

    void VariableValueType4() {
        GET_SYMBOL_DB("class C {\n"
                      "public:\n"
                      "  std::shared_ptr<C> x;\n"
                      "};");

        const Variable* const x = db->getVariableFromVarId(1);
        ASSERT(x->valueType());
        ASSERT(x->valueType()->smartPointerType);
    }

    void VariableValueType5() {
        GET_SYMBOL_DB("class C {};\n"
                      "void foo(std::shared_ptr<C>* p) {}");

        const Variable* const p = db->getVariableFromVarId(1);
        ASSERT(p->valueType());
        ASSERT(p->valueType()->smartPointerTypeToken);
        ASSERT(p->valueType()->pointer == 1);
    }

    void VariableValueTypeReferences() {
        {
            GET_SYMBOL_DB("void foo(int x) {}\n");
            const Variable* const p = db->getVariableFromVarId(1);
            ASSERT(p->valueType());
            ASSERT(p->valueType()->pointer == 0);
            ASSERT(p->valueType()->constness == 0);
            ASSERT(p->valueType()->reference == Reference::None);
        }
        {
            GET_SYMBOL_DB("void foo(int* x) {}\n");
            const Variable* const p = db->getVariableFromVarId(1);
            ASSERT(p->valueType());
            ASSERT(p->valueType()->pointer == 1);
            ASSERT(p->valueType()->constness == 0);
            ASSERT(p->valueType()->reference == Reference::None);
        }
        {
            GET_SYMBOL_DB("void foo(int& x) {}\n");
            const Variable* const p = db->getVariableFromVarId(1);
            ASSERT(p->valueType());
            ASSERT(p->valueType()->pointer == 0);
            ASSERT(p->valueType()->constness == 0);
            ASSERT(p->valueType()->reference == Reference::LValue);
        }
        {
            GET_SYMBOL_DB("void foo(int&& x) {}\n");
            const Variable* const p = db->getVariableFromVarId(1);
            ASSERT(p->valueType());
            ASSERT(p->valueType()->pointer == 0);
            ASSERT(p->valueType()->constness == 0);
            ASSERT(p->valueType()->reference == Reference::RValue);
        }
        {
            GET_SYMBOL_DB("void foo(int*& x) {}\n");
            const Variable* const p = db->getVariableFromVarId(1);
            ASSERT(p->valueType());
            ASSERT(p->valueType()->pointer == 1);
            ASSERT(p->valueType()->constness == 0);
            ASSERT(p->valueType()->reference == Reference::LValue);
        }
        {
            GET_SYMBOL_DB("void foo(int*&& x) {}\n");
            const Variable* const p = db->getVariableFromVarId(1);
            ASSERT(p->valueType());
            ASSERT(p->valueType()->pointer == 1);
            ASSERT(p->valueType()->constness == 0);
            ASSERT(p->valueType()->reference == Reference::RValue);
        }
        {
            GET_SYMBOL_DB("void foo(int**& x) {}\n");
            const Variable* const p = db->getVariableFromVarId(1);
            ASSERT(p->valueType());
            ASSERT(p->valueType()->pointer == 2);
            ASSERT(p->valueType()->constness == 0);
            ASSERT(p->valueType()->reference == Reference::LValue);
        }
        {
            GET_SYMBOL_DB("void foo(int**&& x) {}\n");
            const Variable* const p = db->getVariableFromVarId(1);
            ASSERT(p->valueType());
            ASSERT(p->valueType()->pointer == 2);
            ASSERT(p->valueType()->constness == 0);
            ASSERT(p->valueType()->reference == Reference::RValue);
        }
        {
            GET_SYMBOL_DB("void foo(const int& x) {}\n");
            const Variable* const p = db->getVariableFromVarId(1);
            ASSERT(p->valueType());
            ASSERT(p->valueType()->pointer == 0);
            ASSERT(p->valueType()->constness == 1);
            ASSERT(p->valueType()->reference == Reference::LValue);
        }
        {
            GET_SYMBOL_DB("void foo(const int&& x) {}\n");
            const Variable* const p = db->getVariableFromVarId(1);
            ASSERT(p->valueType());
            ASSERT(p->valueType()->pointer == 0);
            ASSERT(p->valueType()->constness == 1);
            ASSERT(p->valueType()->reference == Reference::RValue);
        }
        {
            GET_SYMBOL_DB("void foo(const int*& x) {}\n");
            const Variable* const p = db->getVariableFromVarId(1);
            ASSERT(p->valueType());
            ASSERT(p->valueType()->pointer == 1);
            ASSERT(p->valueType()->constness == 1);
            ASSERT(p->valueType()->reference == Reference::LValue);
        }
        {
            GET_SYMBOL_DB("void foo(const int*&& x) {}\n");
            const Variable* const p = db->getVariableFromVarId(1);
            ASSERT(p->valueType());
            ASSERT(p->valueType()->pointer == 1);
            ASSERT(p->valueType()->constness == 1);
            ASSERT(p->valueType()->reference == Reference::RValue);
        }
        {
            GET_SYMBOL_DB("void foo(int* const & x) {}\n");
            const Variable* const p = db->getVariableFromVarId(1);
            ASSERT(p->valueType());
            ASSERT(p->valueType()->pointer == 1);
            ASSERT(p->valueType()->constness == 2);
            ASSERT(p->valueType()->reference == Reference::LValue);
        }
        {
            GET_SYMBOL_DB("void foo(int* const && x) {}\n");
            const Variable* const p = db->getVariableFromVarId(1);
            ASSERT(p->valueType());
            ASSERT(p->valueType()->pointer == 1);
            ASSERT(p->valueType()->constness == 2);
            ASSERT(p->valueType()->reference == Reference::RValue);
        }
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

    void findVariableTypeExternC() {
        GET_SYMBOL_DB("extern \"C\" { typedef int INT; }\n"
                      "void bar() {\n"
                      "    INT x = 3;\n"
                      "}");
        (void)db;
        const Variable* avar = Token::findsimplematch(tokenizer.tokens(), "x")->variable();
        ASSERT(avar);
        ASSERT(avar->valueType() != nullptr);
        ASSERT(avar->valueType()->str() == "signed int");
    }

    void rangeBasedFor() {
        GET_SYMBOL_DB("void reset() {\n"
                      "    for(auto& e : array)\n"
                      "        foo(e);\n"
                      "}");

        ASSERT(db != nullptr);

        ASSERT(db->scopeList.back().type == Scope::eFor);
        ASSERT_EQUALS(2, db->variableList().size());

        const Variable* e = db->getVariableFromVarId(1);
        ASSERT(e && e->isReference() && e->isLocal());
    }
    void isVariableStlType() {
        {
            reset();
            GET_SYMBOL_DB("std::string s;");
            const bool result = db->scopeList.front().isVariableDeclaration(tokenizer.tokens(), vartok, typetok);
            ASSERT_EQUALS(true, result);
            Variable v(vartok, tokenizer.tokens(), tokenizer.list.back(), 0, AccessControl::Public, nullptr, nullptr, &settings1);
            static const std::set<std::string> types = { "string", "wstring" };
            static const std::set<std::string> no_types = { "set" };
            ASSERT_EQUALS(true, v.isStlType());
            ASSERT_EQUALS(true, v.isStlType(types));
            ASSERT_EQUALS(false, v.isStlType(no_types));
            ASSERT_EQUALS(true, v.isStlStringType());
        }
        {
            reset();
            GET_SYMBOL_DB("std::vector<int> v;");
            const bool result = db->scopeList.front().isVariableDeclaration(tokenizer.tokens(), vartok, typetok);
            ASSERT_EQUALS(true, result);
            Variable v(vartok, tokenizer.tokens(), tokenizer.list.back(), 0, AccessControl::Public, nullptr, nullptr, &settings1);
            static const std::set<std::string> types = { "bitset", "set", "vector", "wstring" };
            static const std::set<std::string> no_types = { "bitset", "map", "set" };
            ASSERT_EQUALS(true, v.isStlType());
            ASSERT_EQUALS(true, v.isStlType(types));
            ASSERT_EQUALS(false, v.isStlType(no_types));
            ASSERT_EQUALS(false, v.isStlStringType());
        }
        {
            reset();
            GET_SYMBOL_DB("SomeClass s;");
            const bool result = db->scopeList.front().isVariableDeclaration(tokenizer.tokens(), vartok, typetok);
            ASSERT_EQUALS(true, result);
            Variable v(vartok, tokenizer.tokens(), tokenizer.list.back(), 0, AccessControl::Public, nullptr, nullptr, &settings1);
            static const std::set<std::string> types = { "bitset", "set", "vector" };
            ASSERT_EQUALS(false, v.isStlType());
            ASSERT_EQUALS(false, v.isStlType(types));
            ASSERT_EQUALS(false, v.isStlStringType());
        }
    }

    void isVariablePointerToConstPointer() {
        reset();
        GET_SYMBOL_DB("char* const * s;");
        const bool result = db->scopeList.front().isVariableDeclaration(tokenizer.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        Variable v(vartok, typetok, vartok->previous(), 0, AccessControl::Public, nullptr, nullptr, &settings1);
        ASSERT(false == v.isArray());
        ASSERT(true == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void isVariablePointerToVolatilePointer() {
        reset();
        GET_SYMBOL_DB("char* volatile * s;");
        const bool result = db->scopeList.front().isVariableDeclaration(tokenizer.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        Variable v(vartok, typetok, vartok->previous(), 0, AccessControl::Public, nullptr, nullptr, &settings1);
        ASSERT(false == v.isArray());
        ASSERT(true == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void isVariablePointerToConstVolatilePointer() {
        reset();
        GET_SYMBOL_DB("char* const volatile * s;");
        const bool result = db->scopeList.front().isVariableDeclaration(tokenizer.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        Variable v(vartok, typetok, vartok->previous(), 0, AccessControl::Public, nullptr, nullptr, &settings1);
        ASSERT(false == v.isArray());
        ASSERT(true == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void isVariableMultiplePointersAndQualifiers() {
        reset();
        GET_SYMBOL_DB("const char* const volatile * const volatile * const volatile * const volatile s;");
        const bool result = db->scopeList.front().isVariableDeclaration(tokenizer.tokens()->next(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        Variable v(vartok, typetok, vartok->previous(), 0, AccessControl::Public, nullptr, nullptr, &settings1);
        ASSERT(false == v.isArray());
        ASSERT(true == v.isPointer());
        ASSERT(false == v.isReference());
    }

    void variableVolatile() {
        GET_SYMBOL_DB("std::atomic<int> x;\n"
                      "volatile int y;");

        const Token *x = Token::findsimplematch(tokenizer.tokens(), "x");
        ASSERT(x);
        ASSERT(x->variable());
        ASSERT(x->variable()->isVolatile());

        const Token *y = Token::findsimplematch(tokenizer.tokens(), "y");
        ASSERT(y);
        ASSERT(y->variable());
        ASSERT(y->variable()->isVolatile());
    }

    void variableConstexpr() {
        GET_SYMBOL_DB("constexpr int x = 16;");

        const Token *x = Token::findsimplematch(tokenizer.tokens(), "x");
        ASSERT(x);
        ASSERT(x->variable());
        ASSERT(x->variable()->isConst());
        ASSERT(x->variable()->isStatic());
        ASSERT(x->valueType());
        ASSERT(x->valueType()->pointer == 0);
        ASSERT(x->valueType()->constness == 1);
        ASSERT(x->valueType()->reference == Reference::None);
    }

    void isVariableDecltype() {
        GET_SYMBOL_DB("int x;\n"
                      "decltype(x) a;\n"
                      "const decltype(x) b;\n"
                      "decltype(x) *c;\n");
        ASSERT(db);
        ASSERT_EQUALS(4, db->scopeList.front().varlist.size());

        const Variable *a = Token::findsimplematch(tokenizer.tokens(), "a")->variable();
        ASSERT(a);
        ASSERT_EQUALS("a", a->name());
        ASSERT(a->valueType());
        ASSERT_EQUALS("signed int", a->valueType()->str());

        const Variable *b = Token::findsimplematch(tokenizer.tokens(), "b")->variable();
        ASSERT(b);
        ASSERT_EQUALS("b", b->name());
        ASSERT(b->valueType());
        ASSERT_EQUALS("const signed int", b->valueType()->str());

        const Variable *c = Token::findsimplematch(tokenizer.tokens(), "c")->variable();
        ASSERT(c);
        ASSERT_EQUALS("c", c->name());
        ASSERT(c->valueType());
        ASSERT_EQUALS("signed int *", c->valueType()->str());
    }

    void isVariableAlignas() {
        GET_SYMBOL_DB_C("extern alignas(16) int x;\n"
                        "alignas(16) int x;\n");
        ASSERT(db);
        ASSERT_EQUALS(2, db->scopeList.front().varlist.size());
        const Variable *x1 = Token::findsimplematch(tokenizer.tokens(), "x")->variable();
        ASSERT(x1 && Token::simpleMatch(x1->typeStartToken(), "alignas ( 16 ) int x ;"));
    }

    void memberVar1() {
        GET_SYMBOL_DB("struct Foo {\n"
                      "    int x;\n"
                      "};\n"
                      "struct Bar : public Foo {};\n"
                      "void f() {\n"
                      "    struct Bar bar;\n"
                      "    bar.x = 123;\n"  // <- x should get a variable() pointer
                      "}");

        ASSERT(db != nullptr);
        const Token *tok = Token::findsimplematch(tokenizer.tokens(), "x =");
        ASSERT(tok->variable());
        ASSERT(Token::simpleMatch(tok->variable()->typeStartToken(), "int x ;"));
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

    void arrayMemberVar4() {
        GET_SYMBOL_DB("struct S { unsigned char* s; };\n"
                      "struct T { S s[38]; };\n"
                      "void f(T* t) {\n"
                      "    t->s;\n"
                      "}\n");
        const Token *tok = Token::findsimplematch(tokenizer.tokens(), ". s");
        tok = tok ? tok->next() : nullptr;
        ASSERT(db != nullptr);
        ASSERT(tok && tok->variable() && Token::simpleMatch(tok->variable()->typeStartToken(), "S s [ 38 ] ;"));
        ASSERT(tok && tok->varId() == 4U);
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
                      "int y;");

        const Variable* v = db->getVariableFromVarId(2);
        // three elements: varId 0 also counts via a fake-entry
        ASSERT(v && db->variableList().size() == 3);

        ASSERT_THROW(db->getVariableFromVarId(3), std::out_of_range);
    }

    void hasRegularFunction() {
        GET_SYMBOL_DB("void func() { }");

        // 2 scopes: Global and Function
        ASSERT(db && db->scopeList.size() == 2);

        const Scope *scope = findFunctionScopeByToken(db, tokenizer.tokens()->next());

        ASSERT(scope && scope->className == "func");

        ASSERT(scope->functionOf == nullptr);

        const Function *function = findFunctionByName("func", &db->scopeList.front());

        ASSERT(function && function->token->str() == "func");
        ASSERT(function && function->token == tokenizer.tokens()->next());
        ASSERT(function && function->hasBody());
        ASSERT(function && function->functionScope == scope && scope->function == function && function->nestedIn != scope);
        ASSERT(function && function->retDef == tokenizer.tokens());
    }

    void hasRegularFunction_trailingReturnType() {
        GET_SYMBOL_DB("auto func() -> int { }");

        // 2 scopes: Global and Function
        ASSERT(db && db->scopeList.size() == 2);

        const Scope *scope = findFunctionScopeByToken(db, tokenizer.tokens()->next());

        ASSERT(scope && scope->className == "func");

        ASSERT(scope->functionOf == nullptr);

        const Function *function = findFunctionByName("func", &db->scopeList.front());

        ASSERT(function && function->token->str() == "func");
        ASSERT(function && function->token == tokenizer.tokens()->next());
        ASSERT(function && function->hasBody());
        ASSERT(function && function->functionScope == scope && scope->function == function && function->nestedIn != scope);
        ASSERT(function && function->retDef == tokenizer.tokens()->tokAt(5));
    }

    void hasInlineClassFunction() {
        GET_SYMBOL_DB("class Fred { void func() { } };");

        // 3 scopes: Global, Class, and Function
        ASSERT(db && db->scopeList.size() == 3);

        const Token * const functionToken = Token::findsimplematch(tokenizer.tokens(), "func");

        const Scope *scope = findFunctionScopeByToken(db, functionToken);

        ASSERT(scope && scope->className == "func");

        ASSERT(scope->functionOf && scope->functionOf == db->findScopeByName("Fred"));

        const Function *function = findFunctionByName("func", &db->scopeList.back());

        ASSERT(function && function->token->str() == "func");
        ASSERT(function && function->token == functionToken);
        ASSERT(function && function->hasBody() && function->isInline());
        ASSERT(function && function->functionScope == scope && scope->function == function && function->nestedIn == db->findScopeByName("Fred"));
        ASSERT(function && function->retDef == functionToken->previous());

        ASSERT(db && db->findScopeByName("Fred") && db->findScopeByName("Fred")->definedType->getFunction("func") == function);
    }


    void hasInlineClassFunction_trailingReturnType() {
        GET_SYMBOL_DB("class Fred { auto func() -> int { } };");

        // 3 scopes: Global, Class, and Function
        ASSERT(db && db->scopeList.size() == 3);

        const Token * const functionToken = Token::findsimplematch(tokenizer.tokens(), "func");

        const Scope *scope = findFunctionScopeByToken(db, functionToken);

        ASSERT(scope && scope->className == "func");

        ASSERT(scope->functionOf && scope->functionOf == db->findScopeByName("Fred"));

        const Function *function = findFunctionByName("func", &db->scopeList.back());

        ASSERT(function && function->token->str() == "func");
        ASSERT(function && function->token == functionToken);
        ASSERT(function && function->hasBody() && function->isInline());
        ASSERT(function && function->functionScope == scope && scope->function == function && function->nestedIn == db->findScopeByName("Fred"));
        ASSERT(function && function->retDef == functionToken->tokAt(4));

        ASSERT(db && db->findScopeByName("Fred") && db->findScopeByName("Fred")->definedType->getFunction("func") == function);
    }

    void hasMissingInlineClassFunction() {
        GET_SYMBOL_DB("class Fred { void func(); };");

        // 2 scopes: Global and Class (no Function scope because there is no function implementation)
        ASSERT(db && db->scopeList.size() == 2);

        const Token * const functionToken = Token::findsimplematch(tokenizer.tokens(), "func");

        const Scope *scope = findFunctionScopeByToken(db, functionToken);

        ASSERT(scope == nullptr);

        const Function *function = findFunctionByName("func", &db->scopeList.back());

        ASSERT(function && function->token->str() == "func");
        ASSERT(function && function->token == functionToken);
        ASSERT(function && !function->hasBody());
    }

    void hasInlineClassOperatorTemplate() {
        GET_SYMBOL_DB("struct Fred { template<typename T> Foo & operator=(const Foo &) { return *this; } };");

        // 3 scopes: Global, Class, and Function
        ASSERT(db && db->scopeList.size() == 3);

        const Token * const functionToken = Token::findsimplematch(tokenizer.tokens(), "operator=");

        const Scope *scope = findFunctionScopeByToken(db, functionToken);

        ASSERT(scope && scope->className == "operator=");

        ASSERT(scope->functionOf && scope->functionOf == db->findScopeByName("Fred"));

        const Function *function = findFunctionByName("operator=", &db->scopeList.back());

        ASSERT(function && function->token->str() == "operator=");
        ASSERT(function && function->token == functionToken);
        ASSERT(function && function->hasBody() && function->isInline());
        ASSERT(function && function->functionScope == scope && scope->function == function && function->nestedIn == db->findScopeByName("Fred"));
        ASSERT(function && function->retDef == functionToken->tokAt(-2));

        ASSERT(db && db->findScopeByName("Fred") && db->findScopeByName("Fred")->definedType->getFunction("operator=") == function);
    }

    void hasClassFunction() {
        GET_SYMBOL_DB("class Fred { void func(); }; void Fred::func() { }");

        // 3 scopes: Global, Class, and Function
        ASSERT(db && db->scopeList.size() == 3);

        const Token * const functionToken = Token::findsimplematch(tokenizer.tokens()->linkAt(2), "func");

        const Scope *scope = findFunctionScopeByToken(db, functionToken);

        ASSERT(scope && scope->className == "func");

        ASSERT(scope->functionOf && scope->functionOf == db->findScopeByName("Fred"));

        const Function *function = findFunctionByName("func", &db->scopeList.back());

        ASSERT(function && function->token->str() == "func");
        ASSERT(function && function->token == functionToken);
        ASSERT(function && function->hasBody() && !function->isInline());
        ASSERT(function && function->functionScope == scope && scope->function == function && function->nestedIn == db->findScopeByName("Fred"));
    }

    void hasClassFunction_trailingReturnType() {
        GET_SYMBOL_DB("class Fred { auto func() -> int; }; auto Fred::func() -> int { }");

        // 3 scopes: Global, Class, and Function
        ASSERT(db && db->scopeList.size() == 3);

        const Token * const functionToken = Token::findsimplematch(tokenizer.tokens()->linkAt(2), "func");

        const Scope *scope = findFunctionScopeByToken(db, functionToken);

        ASSERT(scope && scope->className == "func");

        ASSERT(scope->functionOf && scope->functionOf == db->findScopeByName("Fred"));

        const Function *function = findFunctionByName("func", &db->scopeList.back());

        ASSERT(function && function->token->str() == "func");
        ASSERT(function && function->token == functionToken);
        ASSERT(function && function->hasBody() && !function->isInline());
        ASSERT(function && function->functionScope == scope && scope->function == function && function->nestedIn == db->findScopeByName("Fred"));
    }

    void hasClassFunction_decltype_auto()
    {
        GET_SYMBOL_DB("struct d { decltype(auto) f() {} };");

        // 3 scopes: Global, Class, and Function
        ASSERT(db && db->scopeList.size() == 3);

        const Token* const functionToken = Token::findsimplematch(tokenizer.tokens(), "f");

        const Scope* scope = findFunctionScopeByToken(db, functionToken);

        ASSERT(scope && scope->className == "f");

        ASSERT(scope->functionOf && scope->functionOf == db->findScopeByName("d"));

        const Function* function = findFunctionByName("f", &db->scopeList.back());

        ASSERT(function && function->token->str() == "f");
        ASSERT(function && function->token == functionToken);
        ASSERT(function && function->hasBody());
    }

    void hasRegularFunctionReturningFunctionPointer() {
        GET_SYMBOL_DB("void (*func(int f))(char) { }");

        // 2 scopes: Global and Function
        ASSERT(db && db->scopeList.size() == 2);

        const Token * const functionToken = Token::findsimplematch(tokenizer.tokens(), "func");

        const Scope *scope = findFunctionScopeByToken(db, functionToken);

        ASSERT(scope && scope->className == "func");

        const Function *function = findFunctionByName("func", &db->scopeList.front());

        ASSERT(function && function->token->str() == "func");
        ASSERT(function && function->token == functionToken);
        ASSERT(function && function->hasBody());
    }

    void hasInlineClassFunctionReturningFunctionPointer() {
        GET_SYMBOL_DB("class Fred { void (*func(int f))(char) { } };");

        // 3 scopes: Global, Class, and Function
        ASSERT(db && db->scopeList.size() == 3);

        const Token * const functionToken = Token::findsimplematch(tokenizer.tokens(), "func");

        const Scope *scope = findFunctionScopeByToken(db, functionToken);

        ASSERT(scope && scope->className == "func");

        const Function *function = findFunctionByName("func", &db->scopeList.back());

        ASSERT(function && function->token->str() == "func");
        ASSERT(function && function->token == functionToken);
        ASSERT(function && function->hasBody() && function->isInline());
    }

    void hasMissingInlineClassFunctionReturningFunctionPointer() {
        GET_SYMBOL_DB("class Fred { void (*func(int f))(char); };");

        // 2 scopes: Global and Class (no Function scope because there is no function implementation)
        ASSERT(db && db->scopeList.size() == 2);

        const Token * const functionToken = Token::findsimplematch(tokenizer.tokens(), "func");

        const Scope *scope = findFunctionScopeByToken(db, functionToken);

        ASSERT(scope == nullptr);

        const Function *function = findFunctionByName("func", &db->scopeList.back());

        ASSERT(function && function->token->str() == "func");
        ASSERT(function && function->token == functionToken);
        ASSERT(function && !function->hasBody());
    }

    void hasClassFunctionReturningFunctionPointer() {
        GET_SYMBOL_DB("class Fred { void (*func(int f))(char); }; void (*Fred::func(int f))(char) { }");

        // 3 scopes: Global, Class, and Function
        ASSERT(db && db->scopeList.size() == 3);

        const Token * const functionToken = Token::findsimplematch(tokenizer.tokens()->linkAt(2), "func");

        const Scope *scope = findFunctionScopeByToken(db, functionToken);

        ASSERT(scope && scope->className == "func");

        const Function *function = findFunctionByName("func", &db->scopeList.back());

        ASSERT(function && function->token->str() == "func");
        ASSERT(function && function->token == functionToken);
        ASSERT(function && function->hasBody() && !function->isInline());
    }

    void methodWithRedundantScope() {
        GET_SYMBOL_DB("class Fred { void Fred::func() {} };");

        // 3 scopes: Global, Class, and Function
        ASSERT(db && db->scopeList.size() == 3);

        const Token * const functionToken = Token::findsimplematch(tokenizer.tokens(), "func");

        const Scope *scope = findFunctionScopeByToken(db, functionToken);

        ASSERT(scope && scope->className == "func");

        const Function *function = findFunctionByName("func", &db->scopeList.back());

        ASSERT(function && function->token->str() == "func");
        ASSERT(function && function->token == functionToken);
        ASSERT(function && function->hasBody() && function->isInline());
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

        ASSERT_EQUALS(10, db->variableList().size() - 1);
        ASSERT_EQUALS(true, db->getVariableFromVarId(1) && db->getVariableFromVarId(1)->dimensions().size() == 1);
        ASSERT_EQUALS(true, db->getVariableFromVarId(2) != nullptr);
        // NOLINTNEXTLINE(readability-container-size-empty)
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

    void pointerToMemberFunction() {
        GET_SYMBOL_DB("bool (A::*pFun)();"); // Pointer to member function of A, returning bool and taking no parameters

        ASSERT(db != nullptr);

        ASSERT_EQUALS(1, db->variableList().size() - 1);
        ASSERT_EQUALS(true, db->getVariableFromVarId(1) != nullptr);

        ASSERT_EQUALS("pFun", db->getVariableFromVarId(1)->name());
        ASSERT_EQUALS("", errout.str());
    }

    void hasSubClassConstructor() {
        GET_SYMBOL_DB("class Foo { class Sub; }; class Foo::Sub { Sub() {} };");
        ASSERT(db != nullptr);

        bool seen_something = false;
        for (const Scope & scope : db->scopeList) {
            for (std::list<Function>::const_iterator func = scope.functionList.cbegin(); func != scope.functionList.cend(); ++func) {
                ASSERT_EQUALS("Sub", func->token->str());
                ASSERT_EQUALS(true, func->hasBody());
                ASSERT_EQUALS(Function::eConstructor, func->type);
                seen_something = true;
            }
        }
        ASSERT_EQUALS(true, seen_something);
    }

    void testConstructors() {
        {
            GET_SYMBOL_DB("class Foo { Foo(); };");
            const Function* ctor = tokenizer.tokens()->tokAt(3)->function();
            ASSERT(db && ctor && ctor->type == Function::eConstructor);
            ASSERT(ctor && ctor->retDef == nullptr);
        }
        {
            GET_SYMBOL_DB("class Foo { Foo(Foo f); };");
            const Function* ctor = tokenizer.tokens()->tokAt(3)->function();
            ASSERT(db && ctor && ctor->type == Function::eConstructor && !ctor->isExplicit());
            ASSERT(ctor && ctor->retDef == nullptr);
        }
        {
            GET_SYMBOL_DB("class Foo { explicit Foo(Foo f); };");
            const Function* ctor = tokenizer.tokens()->tokAt(4)->function();
            ASSERT(db && ctor && ctor->type == Function::eConstructor && ctor->isExplicit());
            ASSERT(ctor && ctor->retDef == nullptr);
        }
        {
            GET_SYMBOL_DB("class Foo { Foo(Bar& f); };");
            const Function* ctor = tokenizer.tokens()->tokAt(3)->function();
            ASSERT(db && ctor && ctor->type == Function::eConstructor);
            ASSERT(ctor && ctor->retDef == nullptr);
        }
        {
            GET_SYMBOL_DB("class Foo { Foo(Foo& f); };");
            const Function* ctor = tokenizer.tokens()->tokAt(3)->function();
            ASSERT(db && ctor && ctor->type == Function::eCopyConstructor);
            ASSERT(ctor && ctor->retDef == nullptr);
        }
        {
            GET_SYMBOL_DB("class Foo { Foo(const Foo &f); };");
            const Function* ctor = tokenizer.tokens()->tokAt(3)->function();
            ASSERT(db && ctor && ctor->type == Function::eCopyConstructor);
            ASSERT(ctor && ctor->retDef == nullptr);
        }
        {
            GET_SYMBOL_DB("template <T> class Foo { Foo(Foo<T>& f); };");
            const Function* ctor = tokenizer.tokens()->tokAt(7)->function();
            ASSERT(db && ctor && ctor->type == Function::eCopyConstructor);
            ASSERT(ctor && ctor->retDef == nullptr);
        }
        {
            GET_SYMBOL_DB("class Foo { Foo(Foo& f, int default = 0); };");
            const Function* ctor = tokenizer.tokens()->tokAt(3)->function();
            ASSERT(db && ctor && ctor->type == Function::eCopyConstructor);
            ASSERT(ctor && ctor->retDef == nullptr);
        }
        {
            GET_SYMBOL_DB("class Foo { Foo(Foo& f, char noDefault); };");
            const Function* ctor = tokenizer.tokens()->tokAt(3)->function();
            ASSERT(db && ctor && ctor->type == Function::eConstructor);
            ASSERT(ctor && ctor->retDef == nullptr);
        }
        {
            GET_SYMBOL_DB("class Foo { Foo(Foo&& f); };");
            const Function* ctor = tokenizer.tokens()->tokAt(3)->function();
            ASSERT(db && ctor && ctor->type == Function::eMoveConstructor);
            ASSERT(ctor && ctor->retDef == nullptr);
        }
        {
            GET_SYMBOL_DB("class Foo { Foo(Foo&& f, int default = 1, bool defaultToo = true); };");
            const Function* ctor = tokenizer.tokens()->tokAt(3)->function();
            ASSERT(db && ctor && ctor->type == Function::eMoveConstructor);
            ASSERT(ctor && ctor->retDef == nullptr);
        }
    }

    void functionDeclarationTemplate() {
        GET_SYMBOL_DB("std::map<int, string> foo() {}");

        // 2 scopes: Global and Function
        ASSERT(db && db->scopeList.size() == 2 && findFunctionByName("foo", &db->scopeList.back()));

        const Scope *scope = &db->scopeList.front();

        ASSERT(scope && scope->functionList.size() == 1);

        const Function *foo = &scope->functionList.front();

        ASSERT(foo && foo->token->str() == "foo");
        ASSERT(foo && foo->hasBody());
    }

    void functionDeclarations() {
        GET_SYMBOL_DB("void foo();\nvoid foo();\nint foo(int i);\nvoid foo() {}");

        // 2 scopes: Global and Function
        ASSERT(db && db->scopeList.size() == 2 && findFunctionByName("foo", &db->scopeList.back()));

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

    void functionDeclarations2() {
        const Settings settingsOld = settings1;
        GET_SYMBOL_DB_STD("std::array<int,2> foo(int x);");

        // 1 scopes: Global
        ASSERT(db && db->scopeList.size() == 1);

        const Scope *scope = &db->scopeList.front();

        ASSERT(scope && scope->functionList.size() == 1);

        const Function *foo = &scope->functionList.front();

        ASSERT(foo);
        ASSERT(foo->tokenDef->str() == "foo");
        ASSERT(!foo->hasBody());

        const Token*parenthesis = foo->tokenDef->next();
        ASSERT(parenthesis->str() == "(" && parenthesis->previous()->str() == "foo");
        ASSERT(parenthesis->valueType()->type == ValueType::Type::CONTAINER);

        settings1 = settingsOld;
    }

    void constexprFunction() {
        const Settings settingsOld = settings1;
        GET_SYMBOL_DB_STD("constexpr int foo();");

        // 1 scopes: Global
        ASSERT(db && db->scopeList.size() == 1);

        const Scope *scope = &db->scopeList.front();

        ASSERT(scope && scope->functionList.size() == 1);

        const Function *foo = &scope->functionList.front();

        ASSERT(foo);
        ASSERT(foo->tokenDef->str() == "foo");
        ASSERT(!foo->hasBody());
        ASSERT(foo->isConstexpr());

        settings1 = settingsOld;
    }

    void constructorInitialization() {
        GET_SYMBOL_DB("std::string logfile;\n"
                      "std::ofstream log(logfile.c_str(), std::ios::out);");

        // 1 scope: Global
        ASSERT(db && db->scopeList.size() == 1);

        // No functions
        ASSERT(db->scopeList.front().functionList.empty());
    }

    void memberFunctionOfUnknownClassMacro1() {
        GET_SYMBOL_DB("class ScVbaFormatCondition { OUString getServiceImplName() SAL_OVERRIDE; };\n"
                      "void ScVbaValidation::getFormula1() {\n"
                      "    sal_uInt16 nFlags = 0;\n"
                      "    if (pDocSh && !getCellRangesForAddress(nFlags)) ;\n"
                      "}");

        ASSERT(db && errout.str().empty());

        const Scope *scope = db->findScopeByName("getFormula1");
        ASSERT(scope != nullptr);
        ASSERT(scope && scope->nestedIn == &db->scopeList.front());
    }

    void memberFunctionOfUnknownClassMacro2() {
        GET_SYMBOL_DB("class ScVbaFormatCondition { OUString getServiceImplName() SAL_OVERRIDE {} };\n"
                      "void getFormula1() {\n"
                      "    sal_uInt16 nFlags = 0;\n"
                      "    if (pDocSh && !getCellRangesForAddress(nFlags)) ;\n"
                      "}");

        ASSERT(db && errout.str().empty());

        const Scope *scope = db->findScopeByName("getFormula1");
        ASSERT(scope != nullptr);
        ASSERT(scope && scope->nestedIn == &db->scopeList.front());

        scope = db->findScopeByName("getServiceImplName");
        ASSERT(scope != nullptr);
        ASSERT(scope && scope->nestedIn && scope->nestedIn->className == "ScVbaFormatCondition");
    }

    void memberFunctionOfUnknownClassMacro3() {
        GET_SYMBOL_DB("class ScVbaFormatCondition { OUString getServiceImplName() THROW(whatever); };\n"
                      "void ScVbaValidation::getFormula1() {\n"
                      "    sal_uInt16 nFlags = 0;\n"
                      "    if (pDocSh && !getCellRangesForAddress(nFlags)) ;\n"
                      "}");

        ASSERT(db && errout.str().empty());

        const Scope *scope = db->findScopeByName("getFormula1");
        ASSERT(scope != nullptr);
        ASSERT(scope && scope->nestedIn == &db->scopeList.front());
    }

    void functionLinkage() {
        GET_SYMBOL_DB("static void f1() { }\n"
                      "void f2();\n"
                      "extern void f3();\n"
                      "void f4();\n"
                      "extern void f5() { };\n"
                      "void f6() { }");

        ASSERT(db && errout.str().empty());

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

    void classWithFriend() {
        GET_SYMBOL_DB("class Foo {}; class Bar1 { friend class Foo; }; class Bar2 { friend Foo; };");
        // 3 scopes: Global, 3 classes
        ASSERT(db && db->scopeList.size() == 4);

        const Scope* foo = db->findScopeByName("Foo");
        ASSERT(foo != nullptr);
        const Scope* bar1 = db->findScopeByName("Bar1");
        ASSERT(bar1 != nullptr);
        const Scope* bar2 = db->findScopeByName("Bar2");
        ASSERT(bar2 != nullptr);

        ASSERT(bar1->definedType->friendList.size() == 1 && bar1->definedType->friendList.front().nameEnd->str() == "Foo" && bar1->definedType->friendList.front().type == foo->definedType);
        ASSERT(bar2->definedType->friendList.size() == 1 && bar2->definedType->friendList.front().nameEnd->str() == "Foo" && bar2->definedType->friendList.front().type == foo->definedType);
    }

    void parseFunctionCorrect() {
        // ticket 3188 - "if" statement parsed as function
        GET_SYMBOL_DB("void func(i) int i; { if (i == 1) return; }");
        ASSERT(db != nullptr);

        // 3 scopes: Global, function, if
        ASSERT_EQUALS(3, db->scopeList.size());

        ASSERT(findFunctionByName("func", &db->scopeList.back()) != nullptr);
        ASSERT(findFunctionByName("if", &db->scopeList.back()) == nullptr);
    }

    void parseFunctionDeclarationCorrect() {
        GET_SYMBOL_DB("void func();\n"
                      "int bar() {}\n"
                      "void func() {}");
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
        GET_SYMBOL_DB("int i;");

        ASSERT(db && db->scopeList.size() == 1);

        const std::list<Scope>::const_iterator it = db->scopeList.cbegin();
        ASSERT(it->varlist.size() == 1);
        const std::list<Variable>::const_iterator var = it->varlist.cbegin();
        ASSERT(var->name() == "i");
        ASSERT(var->typeStartToken()->str() == "int");
    }

    void hasGlobalVariables2() {
        GET_SYMBOL_DB("int array[2][2];");

        ASSERT(db && db->scopeList.size() == 1);

        const std::list<Scope>::const_iterator it = db->scopeList.cbegin();
        ASSERT(it->varlist.size() == 1);

        const std::list<Variable>::const_iterator var = it->varlist.cbegin();
        ASSERT(var->name() == "array");
        ASSERT(var->typeStartToken()->str() == "int");
    }

    void hasGlobalVariables3() {
        GET_SYMBOL_DB("int array[2][2] = { { 0, 0 }, { 0, 0 } };");

        ASSERT(db && db->scopeList.size() == 1);

        const std::list<Scope>::const_iterator it = db->scopeList.cbegin();
        ASSERT(it->varlist.size() == 1);

        const std::list<Variable>::const_iterator var = it->varlist.cbegin();
        ASSERT(var->name() == "array");
        ASSERT(var->typeStartToken()->str() == "int");
    }

    void checkTypeStartEndToken1() {
        GET_SYMBOL_DB("static std::string i;\n"
                      "static const std::string j;\n"
                      "const std::string* k;\n"
                      "const char m[];\n"
                      "void f(const char* const l;) {}");

        ASSERT(db && db->variableList().size() == 6 && db->getVariableFromVarId(1) && db->getVariableFromVarId(2) && db->getVariableFromVarId(3) && db->getVariableFromVarId(4) && db->getVariableFromVarId(5));

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

        const Function * const f = db->functionScopes.front()->function;
        ASSERT_EQUALS(1U, f->argCount());
        ASSERT_EQUALS(0U, f->initializedArgCount());
        ASSERT_EQUALS(1U, f->minArgCount());
        const Variable * const arg1 = f->getArgumentVar(0);
        ASSERT_EQUALS("char", arg1->typeStartToken()->str());
        ASSERT_EQUALS("char", arg1->typeEndToken()->str());
    }

#define check(...) check_(__FILE__, __LINE__, __VA_ARGS__)
    void check_(const char* file, int line, const char code[], bool debug = true, const char filename[] = "test.cpp") {
        // Clear the error log
        errout.str("");

        // Check..
        const Settings settings = settingsBuilder(settings1).debugwarnings(debug).build();

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        ASSERT_LOC(tokenizer.tokenize(istr, filename), file, line);

        // force symbol database creation
        tokenizer.createSymbolDatabase();
    }

    void functionArgs1() {
        {
            GET_SYMBOL_DB("void f(std::vector<std::string>, const std::vector<int> & v) { }");
            ASSERT_EQUALS(1+1, db->variableList().size());
            const Variable* v = db->getVariableFromVarId(1);
            ASSERT(v && v->isReference() && v->isConst() && v->isArgument());
            const Scope* f = db->findScopeByName("f");
            ASSERT(f && f->type == Scope::eFunction && f->function);

            ASSERT(f->function->argumentList.size() == 2 && f->function->argumentList.front().index() == 0 && f->function->argumentList.front().name().empty() && f->function->argumentList.back().index() == 1);
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

    void functionArgs4() {
        GET_SYMBOL_DB("void f1(char [10], struct foo [10]);");
        ASSERT_EQUALS(true, db->scopeList.front().functionList.size() == 1UL);
        const Function *func = &db->scopeList.front().functionList.front();
        ASSERT_EQUALS(true, func && func->argumentList.size() == 2UL);

        const Variable *first = &func->argumentList.front();
        ASSERT_EQUALS(0UL, first->name().size());
        ASSERT_EQUALS(1UL, first->dimensions().size());
        ASSERT_EQUALS(10UL, first->dimension(0));
        const Variable *second = &func->argumentList.back();
        ASSERT_EQUALS(0UL, second->name().size());
        ASSERT_EQUALS(1UL, second->dimensions().size());
        ASSERT_EQUALS(10UL, second->dimension(0));
    }

    void functionArgs5() { // #7650
        GET_SYMBOL_DB("class ABC {};\n"
                      "class Y {\n"
                      "  enum ABC {A,B,C};\n"
                      "  void f(enum ABC abc) {}\n"
                      "};");
        ASSERT_EQUALS(true, db != nullptr);
        const Token *f = Token::findsimplematch(tokenizer.tokens(), "f ( enum");
        ASSERT_EQUALS(true, f && f->function());
        const Function *func = f->function();
        ASSERT_EQUALS(true, func->argumentList.size() == 1 && func->argumentList.front().type());
        const Type * type = func->argumentList.front().type();
        ASSERT_EQUALS(true, type->isEnumType());
    }

    void functionArgs6() { // #7651
        GET_SYMBOL_DB("class ABC {};\n"
                      "class Y {\n"
                      "  enum ABC {A,B,C};\n"
                      "  void f(ABC abc) {}\n"
                      "};");
        ASSERT_EQUALS(true, db != nullptr);
        const Token *f = Token::findsimplematch(tokenizer.tokens(), "f ( ABC");
        ASSERT_EQUALS(true, f && f->function());
        const Function *func = f->function();
        ASSERT_EQUALS(true, func->argumentList.size() == 1 && func->argumentList.front().type());
        const Type * type = func->argumentList.front().type();
        ASSERT_EQUALS(true, type->isEnumType());
    }

    void functionArgs7() { // #7652
        {
            GET_SYMBOL_DB("struct AB { int a; int b; };\n"
                          "int foo(struct AB *ab);\n"
                          "void bar() {\n"
                          "  struct AB ab;\n"
                          "  foo(&ab);\n"
                          "};");
            ASSERT_EQUALS(true, db != nullptr);
            const Token *f = Token::findsimplematch(tokenizer.tokens(), "foo ( & ab");
            ASSERT_EQUALS(true, f && f->function());
            const Function *func = f->function();
            ASSERT_EQUALS(true, func->tokenDef->linenr() == 2 && func->argumentList.size() == 1 && func->argumentList.front().type());
            const Type * type = func->argumentList.front().type();
            ASSERT_EQUALS(true, type->classDef->linenr() == 1);
        }
        {
            GET_SYMBOL_DB("struct AB { int a; int b; };\n"
                          "int foo(AB *ab);\n"
                          "void bar() {\n"
                          "  struct AB ab;\n"
                          "  foo(&ab);\n"
                          "};");
            ASSERT_EQUALS(true, db != nullptr);
            const Token *f = Token::findsimplematch(tokenizer.tokens(), "foo ( & ab");
            ASSERT_EQUALS(true, f && f->function());
            const Function *func = f->function();
            ASSERT_EQUALS(true, func->tokenDef->linenr() == 2 && func->argumentList.size() == 1 && func->argumentList.front().type());
            const Type * type = func->argumentList.front().type();
            ASSERT_EQUALS(true, type->classDef->linenr() == 1);
        }
        {
            GET_SYMBOL_DB("struct AB { int a; int b; };\n"
                          "int foo(struct AB *ab);\n"
                          "void bar() {\n"
                          "  AB ab;\n"
                          "  foo(&ab);\n"
                          "};");
            ASSERT_EQUALS(true, db != nullptr);
            const Token *f = Token::findsimplematch(tokenizer.tokens(), "foo ( & ab");
            ASSERT_EQUALS(true, f && f->function());
            const Function *func = f->function();
            ASSERT_EQUALS(true, func->tokenDef->linenr() == 2 && func->argumentList.size() == 1 && func->argumentList.front().type());
            const Type * type = func->argumentList.front().type();
            ASSERT_EQUALS(true, type->classDef->linenr() == 1);
        }
        {
            GET_SYMBOL_DB("struct AB { int a; int b; };\n"
                          "int foo(AB *ab);\n"
                          "void bar() {\n"
                          "  AB ab;\n"
                          "  foo(&ab);\n"
                          "};");
            ASSERT_EQUALS(true, db != nullptr);
            const Token *f = Token::findsimplematch(tokenizer.tokens(), "foo ( & ab");
            ASSERT_EQUALS(true, f && f->function());
            const Function *func = f->function();
            ASSERT_EQUALS(true, func->tokenDef->linenr() == 2 && func->argumentList.size() == 1 && func->argumentList.front().type());
            const Type * type = func->argumentList.front().type();
            ASSERT_EQUALS(true, type->classDef->linenr() == 1);
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
        const Token *f = Token::findsimplematch(tokenizer.tokens(), "foo ( b");
        ASSERT_EQUALS(true, f && f->function());
        const Function *func = f->function();
        ASSERT_EQUALS(true, func->tokenDef->linenr() == 4 && func->argumentList.size() == 1 && func->argumentList.front().type());
        const Type * type = func->argumentList.front().type();
        ASSERT_EQUALS(true, type->isStructType());
    }

    void functionArgs9() { // #7657
        GET_SYMBOL_DB("struct A {\n"
                      "  struct B {\n"
                      "    enum C { };\n"
                      "  };\n"
                      "};\n"
                      "void foo(A::B::C c) { }");
        ASSERT_EQUALS(true, db != nullptr);
        const Token *f = Token::findsimplematch(tokenizer.tokens(), "foo (");
        ASSERT_EQUALS(true, f && f->function());
        const Function *func = f->function();
        ASSERT_EQUALS(true, func->argumentList.size() == 1 && func->argumentList.front().type());
        const Type * type = func->argumentList.front().type();
        ASSERT_EQUALS(true, type->isEnumType());
    }

    void functionArgs10() {
        GET_SYMBOL_DB("class Fred {\n"
                      "public:\n"
                      "  Fred(Whitespace = PRESERVE_WHITESPACE);\n"
                      "};\n"
                      "Fred::Fred(Whitespace whitespace) { }");
        ASSERT_EQUALS(true, db != nullptr);
        ASSERT_EQUALS(3, db->scopeList.size());
        std::list<Scope>::const_iterator scope = db->scopeList.cbegin();
        ++scope;
        ASSERT_EQUALS((unsigned int)Scope::eClass, (unsigned int)scope->type);
        ASSERT_EQUALS(1, scope->functionList.size());
        ASSERT(scope->functionList.cbegin()->functionScope != nullptr);
        const Scope * functionScope = scope->functionList.cbegin()->functionScope;
        ++scope;
        ASSERT(functionScope == &*scope);
    }

    void functionArgs11() {
        GET_SYMBOL_DB("class Fred {\n"
                      "public:\n"
                      "  void foo(char a[16]);\n"
                      "};\n"
                      "void Fred::foo(char b[16]) { }");
        ASSERT_EQUALS(true, db != nullptr);
        ASSERT_EQUALS(3, db->scopeList.size());
        std::list<Scope>::const_iterator scope = db->scopeList.cbegin();
        ++scope;
        ASSERT_EQUALS((unsigned int)Scope::eClass, (unsigned int)scope->type);
        ASSERT_EQUALS(1, scope->functionList.size());
        ASSERT(scope->functionList.cbegin()->functionScope != nullptr);
        const Scope * functionScope = scope->functionList.cbegin()->functionScope;
        ++scope;
        ASSERT(functionScope == &*scope);
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
        const Token *f = Token::findsimplematch(tokenizer.tokens(), "foo (");
        ASSERT_EQUALS(true, f && f->function());
        const Function *func = f->function();
        ASSERT_EQUALS(true, func->argumentList.size() == 1 && func->argumentList.front().type());
        const Type * type = func->argumentList.front().type();
        ASSERT_EQUALS(true, type->isEnumType());
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
        const Token *f = Token::findsimplematch(tokenizer.tokens(), "B ( A :: E");
        ASSERT_EQUALS(true, f && f->function());
        const Function *func = f->function();
        ASSERT_EQUALS(true, func->argumentList.size() == 1 && func->argumentList.front().type());
        const Type * type = func->argumentList.front().type();
        ASSERT_EQUALS(true, type->isEnumType() && type->name() == "E");

        f = Token::findsimplematch(tokenizer.tokens(), "B ( A :: S");
        ASSERT_EQUALS(true, f && f->function());
        const Function *func2 = f->function();
        ASSERT_EQUALS(true, func2->argumentList.size() == 1 && func2->argumentList.front().type());
        const Type * type2 = func2->argumentList.front().type();
        ASSERT_EQUALS(true, type2->isStructType() && type2->name() == "S");
    }

    void functionArgs14() { // #7697
        GET_SYMBOL_DB("void f(int (&a)[10], int (&b)[10]);");
        (void)db;
        const Function *func = tokenizer.tokens()->next()->function();
        ASSERT_EQUALS(true, func != nullptr);
        ASSERT_EQUALS(2, func ? func->argCount() : 0);
        ASSERT_EQUALS(0, func ? func->initializedArgCount() : 1);
        ASSERT_EQUALS(2, func ? func->minArgCount() : 0);
    }

    void functionArgs15() { // #7159
        const char code[] =
            "class Class {\n"
            "    void Method(\n"
            "        char c = []()->char {\n"
            "            int d = rand();\n"// the '=' on this line used to reproduce the defect
            "            return d;\n"
            "        }()\n"
            "    );\n"
            "};\n";
        GET_SYMBOL_DB(code);
        ASSERT(db);
        ASSERT_EQUALS(2, db->scopeList.size());
        const Scope& classScope = db->scopeList.back();
        ASSERT_EQUALS(Scope::eClass, classScope.type);
        ASSERT_EQUALS("Class", classScope.className);
        ASSERT_EQUALS(1, classScope.functionList.size());
        const Function& method = classScope.functionList.front();
        ASSERT_EQUALS("Method", method.name());
        ASSERT_EQUALS(1, method.argCount());
        ASSERT_EQUALS(1, method.initializedArgCount());
        ASSERT_EQUALS(0, method.minArgCount());
    }

    void functionArgs16() { // #9591
        const char code[] =
            "struct A { int var; };\n"
            "void foo(int x, decltype(A::var) *&p) {}";
        GET_SYMBOL_DB(code);
        ASSERT(db);
        const Scope *scope = db->functionScopes.front();
        const Function *func = scope->function;
        ASSERT_EQUALS(2, func->argCount());
        const Variable *arg2 = func->getArgumentVar(1);
        ASSERT_EQUALS("p", arg2->name());
        ASSERT(arg2->isPointer());
        ASSERT(arg2->isReference());
    }

    void functionArgs17() {
        const char code[] = "void f(int (*fp)(), int x, int y) {}";
        GET_SYMBOL_DB(code);
        ASSERT(db != nullptr);
        const Scope *scope = db->functionScopes.front();
        const Function *func = scope->function;
        ASSERT_EQUALS(3, func->argCount());
    }

    void functionArgs18() {
        const char code[] = "void f(int (*param1)[2], int param2) {}";
        GET_SYMBOL_DB(code);
        ASSERT(db != nullptr);
        const Scope *scope = db->functionScopes.front();
        const Function *func = scope->function;
        ASSERT_EQUALS(2, func->argCount());
    }

    void functionArgs19() {
        const char code[] = "void f(int (*fp)(int), int x, int y) {}";
        GET_SYMBOL_DB(code);
        ASSERT(db != nullptr);
        const Scope *scope = db->functionScopes.front();
        const Function *func = scope->function;
        ASSERT_EQUALS(3, func->argCount());
    }

    void functionArgs20() { // #11769
        const char code[] = "void f(void *(*g)(void *) = [](void *p) { return p; }) {}";
        GET_SYMBOL_DB(code);
        ASSERT(db != nullptr);
        const Scope *scope = db->functionScopes.front();
        const Function *func = scope->function;
        ASSERT_EQUALS(1, func->argCount());
        const Variable* arg = func->getArgumentVar(0);
        TODO_ASSERT(arg->hasDefault());
    }

    void functionImplicitlyVirtual() {
        GET_SYMBOL_DB("class base { virtual void f(); };\n"
                      "class derived : base { void f(); };\n"
                      "void derived::f() {}");
        ASSERT(db != nullptr);
        ASSERT_EQUALS(4, db->scopeList.size());
        const Function *function = db->scopeList.back().function;
        ASSERT_EQUALS(true, function && function->isImplicitlyVirtual(false));
    }

    void functionGetOverridden() {
        GET_SYMBOL_DB("struct B { virtual void f(); };\n"
                      "struct D : B {\n"
                      "public:\n"
                      "    void f() override;\n"
                      "};\n"
                      "struct D2 : D { void f() override {} };\n");
        ASSERT(db != nullptr);
        ASSERT_EQUALS(5, db->scopeList.size());
        const Function *func = db->scopeList.back().function;
        ASSERT(func && func->nestedIn);
        ASSERT_EQUALS("D2", func->nestedIn->className);
        bool foundAllBaseClasses{};
        const Function* baseFunc = func->getOverriddenFunction(&foundAllBaseClasses);
        ASSERT(baseFunc && baseFunc->nestedIn && foundAllBaseClasses);
        ASSERT_EQUALS("D", baseFunc->nestedIn->className);
    }

    void functionIsInlineKeyword() {
        GET_SYMBOL_DB("inline void fs() {}");
        (void)db;
        const Function *func = db->scopeList.back().function;
        ASSERT(func);
        ASSERT(func->isInlineKeyword());
    }

    void functionStatic() {
        GET_SYMBOL_DB("static void fs() {  }");
        (void)db;
        const Function *func = db->scopeList.back().function;
        ASSERT(func);
        ASSERT(func->isStatic());
    }

    void functionReturnsReference() {
        GET_SYMBOL_DB("Fred::Reference foo();");
        ASSERT_EQUALS(1, db->scopeList.back().functionList.size());
        const Function &func = *db->scopeList.back().functionList.cbegin();
        ASSERT(!Function::returnsReference(&func, false));
        ASSERT(Function::returnsReference(&func, true));
    }

    void namespaces1() {
        GET_SYMBOL_DB("namespace fred {\n"
                      "    namespace barney {\n"
                      "        class X { X(int); };\n"
                      "    }\n"
                      "}\n"
                      "namespace barney { X::X(int) { } }");

        // Locate the scope for the class..
        auto it = std::find_if(db->scopeList.cbegin(), db->scopeList.cend(), [](const Scope& s) {
            return s.isClassOrStruct();
        });
        const Scope *scope = (it == db->scopeList.end()) ? nullptr : &*it;

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
        auto it = std::find_if(db->scopeList.cbegin(), db->scopeList.cend(), [](const Scope& s) {
            return s.isClassOrStruct();
        });
        const Scope* scope = (it == db->scopeList.end()) ? nullptr : &*it;

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

    void needInitialization() { // #10259
        const auto oldSettings = settings1;
        settings1.debugwarnings = true;

        GET_SYMBOL_DB("template <typename T>\n"
                      "struct A {\n"
                      "    using type = T;\n"
                      "    type t_;\n"
                      "};\n");
        ASSERT_EQUALS("", errout.str());

        settings1 = oldSettings;
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
              "public:\n"
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
              "static void function_declaration_after(void) __attribute__((__used__));");
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
        ASSERT_THROW(check("int CL_INLINE_DECL(integer_decode_float) (int x) {\n"
                           "    return (sign ? cl_I() : 0);\n"
                           "}"), InternalError);
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
        check("{return f(){}}");

        ASSERT_EQUALS("", errout.str());
    }

    void symboldatabase19() {
        // ticket #2991 - segmentation fault
        check("::y(){x}");

        ASSERT_EQUALS_WITHOUT_LINENUMBERS("[test.cpp:1]: (debug) Executable scope 'y' with unknown function.\n"
                                          "[test.cpp:1]: (debug) valueflow.cpp:1321:valueFlowConditionExpressions bailout: Skipping function due to incomplete variable x\n", errout.str());
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
              "A<int> a;");
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
        auto it = std::find_if(db->scopeList.cbegin(), db->scopeList.cend(), [&](const Scope& scope) {
            return scope.isClassOrStruct() && scope.className == "Fred";
        });
        const Scope* fredScope = (it == db->scopeList.end()) ? nullptr : &*it;
        ASSERT(fredScope != nullptr);

        // The struct Fred has two functions, a constructor and a destructor
        ASSERT_EQUALS(2U, fredScope->functionList.size());

        // Get linenumbers where the bodies for the constructor and destructor are..
        unsigned int constructor = 0;
        unsigned int destructor = 0;
        for (const Function& f : fredScope->functionList) {
            if (f.type == Function::eConstructor)
                constructor = f.token->linenr();  // line number for constructor body
            if (f.type == Function::eDestructor)
                destructor = f.token->linenr();  // line number for destructor body
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
                      "class Sub;");
        ASSERT(db && db->typeList.size() == 5);

        std::list<Type>::const_iterator i = db->typeList.cbegin();
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
        ASSERT_EQUALS("[test.cpp:2]: (debug) Failed to parse 'typedef _Atomic ( int ) & atomic_int_ref ;'. The checking continues anyway.\n", errout.str());
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
                      "};");
        ASSERT(db && db->typeList.size() == 3);

        std::list<Type>::const_iterator i = db->typeList.cbegin();
        const Type* Fred = &(*i++);
        const Type* Wilma = &(*i++);
        const Type* Barney = &(*i++);
        ASSERT(Fred && Fred->classDef && Fred->classScope && Fred->enclosingScope && Fred->name() == "Fred");
        ASSERT(Wilma && Wilma->classDef && Wilma->classScope && Wilma->enclosingScope && Wilma->name() == "Wilma");
        ASSERT(Barney && Barney->classDef && Barney->classScope && Barney->enclosingScope && Barney->name() == "Barney");
        ASSERT(db->variableList().size() == 5);

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
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void symboldatabase41() { // ticket #5197 (unknown macro)
        GET_SYMBOL_DB("struct X1 { MACRO1 f(int spd) MACRO2; };");
        ASSERT(db && db->findScopeByName("X1") && db->findScopeByName("X1")->functionList.size() == 1 && !db->findScopeByName("X1")->functionList.front().hasBody());
    }

    void symboldatabase42() { // only put variables in variable list
        GET_SYMBOL_DB("void f() { extern int x(); }");
        ASSERT(db != nullptr);
        const Scope * const fscope = db ? db->findScopeByName("f") : nullptr;
        ASSERT(fscope != nullptr);
        ASSERT_EQUALS(0U, fscope ? fscope->varlist.size() : ~0U);  // "x" is not a variable
    }

    void symboldatabase43() { // ticket #4738
        check("void f() {\n"
              "    new int;\n"
              "}");
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
        std::list<Scope>::const_iterator scope = db->scopeList.cbegin();
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
        std::list<Scope>::const_iterator scope = db->scopeList.cbegin();
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
                      "MyClass::MyClass() { }");
        ASSERT(db && !db->functionScopes.empty() && db->functionScopes.front()->function && db->functionScopes.front()->function->functionScope == db->functionScopes.front());

        const Token *f = Token::findsimplematch(tokenizer.tokens(), "MyClass ( ) ;");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 3 && f->function()->token->linenr() == 9);

        f = Token::findsimplematch(tokenizer.tokens(), "~ MyClass ( ) ;");
        f = f->next();
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 4 && f->function()->token->linenr() == 8);
    }

    void symboldatabase49() { // #6424
        GET_SYMBOL_DB("namespace Ns { class C; }\n"
                      "void f1() { char *p; *p = 0; }\n"
                      "class Ns::C* p;\n"
                      "void f2() { char *p; *p = 0; }");
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
                      "static const bool b1 = 2 > foo6(a, b);");
        ASSERT(db != nullptr);
        ASSERT(findFunctionByName("foo1", &db->scopeList.front()) == nullptr);
        ASSERT(findFunctionByName("foo2", &db->scopeList.front()) == nullptr);
        ASSERT(findFunctionByName("foo3", &db->scopeList.front()) == nullptr);
        ASSERT(findFunctionByName("foo4", &db->scopeList.front()) == nullptr);
        ASSERT(findFunctionByName("foo5", &db->scopeList.front()) == nullptr);
        ASSERT(findFunctionByName("foo6", &db->scopeList.front()) == nullptr);
    }

    void symboldatabase52() { // #6581
        GET_SYMBOL_DB("void foo() {\n"
                      "    int i = 0;\n"
                      "    S s{ { i }, 0 };\n"
                      "}");

        ASSERT(db != nullptr);
        ASSERT_EQUALS(2, db->scopeList.size());
        ASSERT_EQUALS(2, db->variableList().size()-1);
        ASSERT(db->getVariableFromVarId(1) != nullptr);
        ASSERT(db->getVariableFromVarId(2) != nullptr);
    }

    void symboldatabase53() { // #7124
        GET_SYMBOL_DB("int32_t x;"
                      "std::int32_t y;");

        ASSERT(db != nullptr);
        ASSERT(db->getVariableFromVarId(1) != nullptr);
        ASSERT(db->getVariableFromVarId(2) != nullptr);
        ASSERT_EQUALS(false, db->getVariableFromVarId(1)->isClass());
        ASSERT_EQUALS(false, db->getVariableFromVarId(2)->isClass());
    }

    void symboldatabase54() { // #7343
        GET_SYMBOL_DB("class A {\n"
                      "  void getReg() const override {\n"
                      "    assert(Kind == k_ShiftExtend);\n"
                      "  }\n"
                      "};");

        ASSERT(db != nullptr);
        ASSERT_EQUALS(1U, db->functionScopes.size());
        ASSERT_EQUALS("getReg", db->functionScopes.front()->className);
        ASSERT_EQUALS(true, db->functionScopes.front()->function->hasOverrideSpecifier());
    }

    void symboldatabase55() { // #7767
        GET_SYMBOL_DB("PRIVATE S32 testfunc(void) {\n"
                      "    return 0;\n"
                      "}");

        ASSERT(db != nullptr);
        ASSERT_EQUALS(1U, db->functionScopes.size());
        ASSERT_EQUALS("testfunc", db->functionScopes.front()->className);
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
            ASSERT_EQUALS(0U, db->functionScopes.size());
            ASSERT(db->scopeList.back().type == Scope::eClass && db->scopeList.back().className == "NestedClass");
            ASSERT(db->scopeList.back().functionList.size() == 1U && !db->scopeList.back().functionList.front().hasBody());
        }
        {
            GET_SYMBOL_DB("class Class {\n"
                          "    friend void f1();\n"
                          "    friend void f2() { }\n"
                          "}");

            ASSERT(db != nullptr);
            ASSERT_EQUALS(1U, db->functionScopes.size());
            ASSERT(db->scopeList.back().type == Scope::eFunction && db->scopeList.back().className == "f2");
            ASSERT(db->scopeList.back().function && db->scopeList.back().function->hasBody());
        }
        {
            GET_SYMBOL_DB_C("friend f1();\n"
                            "friend f2() { }");

            ASSERT(db != nullptr);
            ASSERT_EQUALS(2U, db->scopeList.size());
            ASSERT_EQUALS(2U, db->scopeList.cbegin()->functionList.size());
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
        ASSERT(db->scopeList.size() == 4U);
        std::list<Scope>::const_iterator it = db->scopeList.cbegin();
        ASSERT(it->type == Scope::eGlobal);
        ASSERT((++it)->type == Scope::eFunction);
        ASSERT((++it)->type == Scope::eIf);
        ASSERT((++it)->type == Scope::eElse);
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
        ASSERT(db->typeList.size() == 3U);
        std::list<Type>::const_iterator it = db->typeList.cbegin();
        const Type * classB = &(*it);
        const Type * classC = &(*(++it));
        const Type * classA = &(*(++it));
        ASSERT(classA->name() == "A" && classB->name() == "B" && classC->name() == "C");
        ASSERT(classA->derivedFrom.size() == 1U);
        ASSERT(classA->derivedFrom[0].type != nullptr);
        ASSERT(classA->derivedFrom[0].type == classB);
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
        {
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
            const Scope *scope = db->findScopeByName("A");
            ASSERT(scope != nullptr);
            const Function *function = findFunctionByName("Foo", scope);
            ASSERT(function != nullptr);
            ASSERT(function->hasBody());
        }
        {
            GET_SYMBOL_DB("class A {\n"
                          "public:\n"
                          "    class X { public: int a; };\n"
                          "    void Foo(const std::vector<class X> &includes);\n"
                          "};\n"
                          "void A::Foo(const std::vector<class A::X> &includes) {\n"
                          "    for (std::vector<class A::X>::const_iterator it = includes.begin(); it != includes.end(); ++it) {\n"
                          "        const class A::X currentIncList = *it;\n"
                          "    }\n"
                          "}");
            ASSERT(db != nullptr);
            ASSERT(db && db->scopeList.size() == 5);
            const Scope *scope = db->findScopeByName("A");
            ASSERT(scope != nullptr);
            const Function *function = findFunctionByName("Foo", scope);
            ASSERT(function != nullptr);
            ASSERT(function->hasBody());
        }
        {
            GET_SYMBOL_DB("struct A {\n"
                          "public:\n"
                          "    union X { int a; float b; };\n"
                          "    void Foo(const std::vector<union X> &includes);\n"
                          "};\n"
                          "void A::Foo(const std::vector<union A::X> &includes) {\n"
                          "    for (std::vector<union A::X>::const_iterator it = includes.begin(); it != includes.end(); ++it) {\n"
                          "        const union A::X currentIncList = *it;\n"
                          "    }\n"
                          "}");
            ASSERT(db != nullptr);
            ASSERT(db && db->scopeList.size() == 5);
            const Scope *scope = db->findScopeByName("A");
            ASSERT(scope != nullptr);
            const Function *function = findFunctionByName("Foo", scope);
            ASSERT(function != nullptr);
            ASSERT(function->hasBody());
        }
        {
            GET_SYMBOL_DB("struct A {\n"
                          "public:\n"
                          "    enum X { a, b };\n"
                          "    void Foo(const std::vector<enum X> &includes);\n"
                          "};\n"
                          "void A::Foo(const std::vector<enum A::X> &includes) {\n"
                          "    for (std::vector<enum A::X>::const_iterator it = includes.begin(); it != includes.end(); ++it) {\n"
                          "        const enum A::X currentIncList = *it;\n"
                          "    }\n"
                          "}");
            ASSERT(db != nullptr);
            ASSERT(db && db->scopeList.size() == 5);
            const Scope *scope = db->findScopeByName("A");
            ASSERT(scope != nullptr);
            const Function *function = findFunctionByName("Foo", scope);
            ASSERT(function != nullptr);
            ASSERT(function->hasBody());
        }
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
                          "template <typename A> void Fred<A>::impl::foo(const Fred<A>::impl &, const Fred<A>::impl &) const { }");

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
                          "template <typename A> void NS::Fred<A>::impl::foo(const NS::Fred<A>::impl &, const NS::Fred<A>::impl &) const { }");

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
                          "template <typename A> void NS::Fred<A>::impl::foo(const NS::Fred<A>::impl &, const NS::Fred<A>::impl &) const { }");

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
                          "template <typename A> void Fred<A>::impl::foo(const Fred<A>::impl &, const Fred<A>::impl &) const { }");

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
        ASSERT_EQUALS("", errout.str());

        check("int g();\n" // #11385
              "void f(int i) {\n"
              "    if (i > ::g()) {}\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
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
        const Token *f = db ? Token::findsimplematch(tokenizer.tokens(), "get_endpoint_url ( ) const noexcept ( true ) ;") : nullptr;
        ASSERT(f != nullptr);
        ASSERT(f && f->function() && f->function()->token->linenr() == 2);
        ASSERT(f && f->function() && f->function()->hasVirtualSpecifier());
        ASSERT(f && f->function() && !f->function()->hasOverrideSpecifier());
        ASSERT(f && f->function() && !f->function()->hasFinalSpecifier());
        ASSERT(f && f->function() && f->function()->isConst());
        ASSERT(f && f->function() && f->function()->isNoExcept());
        f = db ? Token::findsimplematch(tokenizer.tokens(), "get_endpoint_url ( ) const noexcept ( true ) override final ;") : nullptr;
        ASSERT(f != nullptr);
        ASSERT(f && f->function() && f->function()->token->linenr() == 5);
        ASSERT(f && f->function() && f->function()->hasVirtualSpecifier());
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

    void symboldatabase74() { // #8838 - final
        GET_SYMBOL_DB("class Base { virtual int f() const = 0; };\n"
                      "class Derived : Base { virtual int f() const final { return 6; } };");

        ASSERT_EQUALS(4, db->scopeList.size());
        ASSERT_EQUALS(1, db->functionScopes.size());

        const Scope *f1 = db->functionScopes[0];
        ASSERT(f1->function->hasFinalSpecifier());
    }

    void symboldatabase75() {
        GET_SYMBOL_DB("template <typename T>\n"
                      "class optional {\n"
                      "  auto     value() & -> T &;\n"
                      "  auto     value() && -> T &&;\n"
                      "  auto     value() const& -> T const &;\n"
                      "};\n"
                      "template <typename T>\n"
                      "auto optional<T>::value() & -> T & {}\n"
                      "template <typename T>\n"
                      "auto optional<T>::value() && -> T && {}\n"
                      "template <typename T>\n"
                      "auto optional<T>::value() const & -> T const & {}\n"
                      "optional<int> i;");

        ASSERT_EQUALS(5, db->scopeList.size());
        ASSERT_EQUALS(3, db->functionScopes.size());

        const Scope *f = db->functionScopes[0];
        ASSERT(f->function->hasBody());
        ASSERT(!f->function->isConst());
        ASSERT(f->function->hasTrailingReturnType());
        ASSERT(f->function->hasLvalRefQualifier());

        f = db->functionScopes[1];
        ASSERT(f->function->hasBody());
        ASSERT(!f->function->isConst());
        ASSERT(f->function->hasTrailingReturnType());
        ASSERT(f->function->hasRvalRefQualifier());

        f = db->functionScopes[2];
        ASSERT(f->function->hasBody());
        ASSERT(f->function->isConst());
        ASSERT(f->function->hasTrailingReturnType());
        ASSERT(f->function->hasLvalRefQualifier());
    }

    void symboldatabase76() { // #9056
        GET_SYMBOL_DB("namespace foo {\n"
                      "  using namespace bar::baz;\n"
                      "  auto func(int arg) -> bar::quux {}\n"
                      "}");
        ASSERT_EQUALS(2, db->mVariableList.size());
    }

    void symboldatabase77() { // #8663
        GET_SYMBOL_DB("template <class T1, class T2>\n"
                      "void f() {\n"
                      "  using T3 = typename T1::template T3<T2>;\n"
                      "  T3 t;\n"
                      "}");
        ASSERT_EQUALS(2, db->mVariableList.size());
    }

    void symboldatabase78() { // #9147
        GET_SYMBOL_DB("template <class...> struct a;\n"
                      "namespace {\n"
                      "template <class, class> struct b;\n"
                      "template <template <class> class c, class... f, template <class...> class d>\n"
                      "struct b<c<f...>, d<>>;\n"
                      "}\n"
                      "void e() { using c = a<>; }");
        ASSERT(db != nullptr);
        ASSERT_EQUALS("", errout.str());
    }

    void symboldatabase79() { // #9392
        {
            GET_SYMBOL_DB("class C { C(); };\n"
                          "C::C() = default;");
            ASSERT(db->scopeList.size() == 2);
            ASSERT(db->scopeList.back().functionList.size() == 1);
            ASSERT(db->scopeList.back().functionList.front().isDefault() == true);
        }
        {
            GET_SYMBOL_DB("namespace ns {\n"
                          "class C { C(); };\n"
                          "}\n"
                          "using namespace ns;\n"
                          "C::C() = default;");
            ASSERT(db->scopeList.size() == 3);
            ASSERT(db->scopeList.back().functionList.size() == 1);
            ASSERT(db->scopeList.back().functionList.front().isDefault() == true);
        }
        {
            GET_SYMBOL_DB("class C { ~C(); };\n"
                          "C::~C() = default;");
            ASSERT(db->scopeList.size() == 2);
            ASSERT(db->scopeList.back().functionList.size() == 1);
            ASSERT(db->scopeList.back().functionList.front().isDefault() == true);
            ASSERT(db->scopeList.back().functionList.front().isDestructor() == true);
        }
        {
            GET_SYMBOL_DB("namespace ns {\n"
                          "class C { ~C(); };\n"
                          "}\n"
                          "using namespace ns;\n"
                          "C::~C() = default;");
            ASSERT(db->scopeList.size() == 3);
            ASSERT(db->scopeList.back().functionList.size() == 1);
            ASSERT(db->scopeList.back().functionList.front().isDefault() == true);
            ASSERT(db->scopeList.back().functionList.front().isDestructor() == true);
        }
    }

    void symboldatabase80() { // #9389
        {
            GET_SYMBOL_DB("namespace ns {\n"
                          "class A {};\n"
                          "}\n"
                          "class AA {\n"
                          "private:\n"
                          "    void f(const ns::A&);\n"
                          "};\n"
                          "using namespace ns;\n"
                          "void AA::f(const A&) { }");
            ASSERT(db->scopeList.size() == 5);
            ASSERT(db->functionScopes.size() == 1);
            const Scope *scope = db->findScopeByName("AA");
            ASSERT(scope);
            ASSERT(scope->functionList.size() == 1);
            ASSERT(scope->functionList.front().name() == "f");
            ASSERT(scope->functionList.front().hasBody() == true);
        }
        {
            GET_SYMBOL_DB("namespace ns {\n"
                          "namespace ns1 {\n"
                          "class A {};\n"
                          "}\n"
                          "}\n"
                          "class AA {\n"
                          "private:\n"
                          "    void f(const ns::ns1::A&);\n"
                          "};\n"
                          "using namespace ns::ns1;\n"
                          "void AA::f(const A&) { }");
            ASSERT(db->scopeList.size() == 6);
            ASSERT(db->functionScopes.size() == 1);
            const Scope *scope = db->findScopeByName("AA");
            ASSERT(scope);
            ASSERT(scope->functionList.size() == 1);
            ASSERT(scope->functionList.front().name() == "f");
            ASSERT(scope->functionList.front().hasBody() == true);
        }
        {
            GET_SYMBOL_DB("namespace ns {\n"
                          "namespace ns1 {\n"
                          "class A {};\n"
                          "}\n"
                          "}\n"
                          "class AA {\n"
                          "private:\n"
                          "    void f(const ns::ns1::A&);\n"
                          "};\n"
                          "using namespace ns;\n"
                          "void AA::f(const ns1::A&) { }");
            ASSERT(db->scopeList.size() == 6);
            ASSERT(db->functionScopes.size() == 1);
            const Scope *scope = db->findScopeByName("AA");
            ASSERT(scope);
            ASSERT(scope->functionList.size() == 1);
            ASSERT(scope->functionList.front().name() == "f");
            ASSERT(scope->functionList.front().hasBody() == true);
        }
    }

    void symboldatabase81() { // #9411
        {
            GET_SYMBOL_DB("namespace Terminal {\n"
                          "    class Complete {\n"
                          "    public:\n"
                          "        std::string act(const Parser::Action *act);\n"
                          "    };\n"
                          "}\n"
                          "using namespace std;\n"
                          "using namespace Parser;\n"
                          "using namespace Terminal;\n"
                          "string Complete::act(const Action *act) { }");
            ASSERT(db->scopeList.size() == 4);
            ASSERT(db->functionScopes.size() == 1);
            const Scope *scope = db->findScopeByName("Complete");
            ASSERT(scope);
            ASSERT(scope->functionList.size() == 1);
            ASSERT(scope->functionList.front().name() == "act");
            ASSERT(scope->functionList.front().hasBody() == true);
        }
        {
            GET_SYMBOL_DB("namespace Terminal {\n"
                          "    class Complete {\n"
                          "    public:\n"
                          "        std::string act(const Foo::Parser::Action *act);\n"
                          "    };\n"
                          "}\n"
                          "using namespace std;\n"
                          "using namespace Foo::Parser;\n"
                          "using namespace Terminal;\n"
                          "string Complete::act(const Action *act) { }");
            ASSERT(db->scopeList.size() == 4);
            ASSERT(db->functionScopes.size() == 1);
            const Scope *scope = db->findScopeByName("Complete");
            ASSERT(scope);
            ASSERT(scope->functionList.size() == 1);
            ASSERT(scope->functionList.front().name() == "act");
            ASSERT(scope->functionList.front().hasBody() == true);
        }
    }

    void symboldatabase82() {
        GET_SYMBOL_DB("namespace foo { void foo() {} }");
        ASSERT(db->functionScopes.size() == 1);
        ASSERT_EQUALS(false, db->functionScopes[0]->function->isConstructor());
    }

    void symboldatabase83() { // #9431
        const Settings settingsOld = settings1;
        settings1.debugwarnings = true;
        GET_SYMBOL_DB("struct a { a() noexcept; };\n"
                      "a::a() noexcept = default;");
        settings1 = settingsOld;
        const Scope *scope = db->findScopeByName("a");
        ASSERT(scope);
        ASSERT(scope->functionList.size() == 1);
        ASSERT(scope->functionList.front().name() == "a");
        ASSERT(scope->functionList.front().hasBody() == false);
        ASSERT(scope->functionList.front().isConstructor() == true);
        ASSERT(scope->functionList.front().isDefault() == true);
        ASSERT(scope->functionList.front().isNoExcept() == true);
        ASSERT_EQUALS("", errout.str());
    }

    void symboldatabase84() {
        {
            const bool old = settings1.debugwarnings;
            settings1.debugwarnings = true;
            GET_SYMBOL_DB("struct a { a() noexcept(false); };\n"
                          "a::a() noexcept(false) = default;");
            settings1.debugwarnings = old;
            const Scope *scope = db->findScopeByName("a");
            ASSERT(scope);
            ASSERT(scope->functionList.size() == 1);
            ASSERT(scope->functionList.front().name() == "a");
            ASSERT(scope->functionList.front().hasBody() == false);
            ASSERT(scope->functionList.front().isConstructor() == true);
            ASSERT(scope->functionList.front().isDefault() == true);
            ASSERT(scope->functionList.front().isNoExcept() == false);
            ASSERT_EQUALS("", errout.str());
        }
        {
            const bool old = settings1.debugwarnings;
            settings1.debugwarnings = true;
            GET_SYMBOL_DB("struct a { a() noexcept(true); };\n"
                          "a::a() noexcept(true) = default;");
            settings1.debugwarnings = old;
            const Scope *scope = db->findScopeByName("a");
            ASSERT(scope);
            ASSERT(scope->functionList.size() == 1);
            ASSERT(scope->functionList.front().name() == "a");
            ASSERT(scope->functionList.front().hasBody() == false);
            ASSERT(scope->functionList.front().isConstructor() == true);
            ASSERT(scope->functionList.front().isDefault() == true);
            ASSERT(scope->functionList.front().isNoExcept() == true);
            ASSERT_EQUALS("", errout.str());
        }
    }

    void symboldatabase85() {
        GET_SYMBOL_DB("class Fred {\n"
                      "  enum Mode { Mode1, Mode2, Mode3 };\n"
                      "  void f() { _mode = x; }\n"
                      "  Mode _mode;\n"
                      "  DECLARE_PROPERTY_FIELD(_mode);\n"
                      "};");
        const Token *vartok1 = Token::findsimplematch(tokenizer.tokens(), "_mode =");
        ASSERT(vartok1);
        ASSERT(vartok1->variable());
        ASSERT(vartok1->variable()->scope());

        const Token *vartok2 = Token::findsimplematch(tokenizer.tokens(), "( _mode ) ;")->next();
        ASSERT_EQUALS(std::intptr_t(vartok1->variable()), std::intptr_t(vartok2->variable()));
    }

    void symboldatabase86() {
        GET_SYMBOL_DB("class C { auto operator=(const C&) -> C&; };\n"
                      "auto C::operator=(const C&) -> C& = default;");
        ASSERT(db->scopeList.size() == 2);
        ASSERT(db->scopeList.back().functionList.size() == 1);
        ASSERT(db->scopeList.back().functionList.front().isDefault() == true);
        ASSERT(db->scopeList.back().functionList.front().hasBody() == false);
    }

    void symboldatabase87() { // #9922 'extern const char ( * x [ 256 ] ) ;'
        GET_SYMBOL_DB("extern const char ( * x [ 256 ] ) ;");
        const Token *xtok = Token::findsimplematch(tokenizer.tokens(), "x");
        ASSERT(xtok->variable());
    }

    void symboldatabase88() { // #10040 (using namespace)
        check("namespace external {\n"
              "namespace ns {\n"
              "enum class s { O };\n"
              "}\n"
              "}\n"
              "namespace internal {\n"
              "namespace ns1 {\n"
              "template <typename T>\n"
              "void make(external::ns::s) {\n"
              "}\n"
              "}\n"
              "}\n"
              "using namespace external::ns;\n"
              "struct A { };\n"
              "static void make(external::ns::s ss) {\n"
              "  internal::ns1::make<A>(ss);\n"
              "}\n", true);
        ASSERT_EQUALS("", errout.str());
    }

    void symboldatabase89() { // valuetype name
        GET_SYMBOL_DB("namespace external {\n"
                      "namespace ns1 {\n"
                      "class A {\n"
                      "public:\n"
                      "  struct S { };\n"
                      "  A(const S&) { }\n"
                      "};\n"
                      "static const A::S AS = A::S();\n"
                      "}\n"
                      "}\n"
                      "using namespace external::ns1;\n"
                      "A a{AS};");
        const Token *vartok1 = Token::findsimplematch(tokenizer.tokens(), "A a");
        ASSERT(vartok1);
        ASSERT(vartok1->next());
        ASSERT(vartok1->next()->variable());
        ASSERT(vartok1->next()->variable()->valueType());
        ASSERT(vartok1->next()->variable()->valueType()->str() == "external::ns1::A");
    }

    void symboldatabase90() {
        GET_SYMBOL_DB("struct Fred {\n"
                      "    void foo(const int * const x);\n"
                      "};\n"
                      "void Fred::foo(const int * x) { }");
        ASSERT_EQUALS("", errout.str());
        const Token *functok = Token::findsimplematch(tokenizer.tokens(), "foo ( const int * x )");
        ASSERT(functok);
        ASSERT(functok->function());
        ASSERT(functok->function()->name() == "foo");
    }

    void symboldatabase91() {
        GET_SYMBOL_DB("namespace Fred {\n"
                      "    struct Value {};\n"
                      "    void foo(const std::vector<std::function<void(const Fred::Value &)>> &callbacks);\n"
                      "}\n"
                      "void Fred::foo(const std::vector<std::function<void(const Fred::Value &)>> &callbacks) { }");
        ASSERT_EQUALS("", errout.str());
        const Token *functok = Token::findsimplematch(tokenizer.tokens(),
                                                      "foo ( const std :: vector < std :: function < void ( const Fred :: Value & ) > > & callbacks ) { }");
        ASSERT(functok);
        ASSERT(functok->function());
        ASSERT(functok->function()->name() == "foo");
    }

    void symboldatabase92() { // daca crash
        {
            GET_SYMBOL_DB("template <size_t, typename...> struct a;\n"
                          "template <size_t b, typename c, typename... d>\n"
                          "struct a<b, c, d...> : a<1, d...> {};\n"
                          "template <typename... e> struct f : a<0, e...> {};");
            ASSERT_EQUALS("", errout.str());
        }
        {
            GET_SYMBOL_DB("b.f();");
            ASSERT_EQUALS("", errout.str());
        }
    }

    void symboldatabase93() { // alignas attribute
        GET_SYMBOL_DB("struct alignas(int) A{\n"
                      "};\n"
                      );
        ASSERT(db != nullptr);
        const Scope* scope = db->findScopeByName("A");
        ASSERT(scope);
    }

    void symboldatabase94() { // structured bindings
        GET_SYMBOL_DB("int foo() { auto [x,y] = xy(); return x+y; }");
        ASSERT(db != nullptr);
        ASSERT(db->getVariableFromVarId(1) != nullptr);
        ASSERT(db->getVariableFromVarId(2) != nullptr);
    }

    void symboldatabase95() { // #10295
        GET_SYMBOL_DB("struct B {\n"
                      "    void foo1(void);\n"
                      "    void foo2();\n"
                      "};\n"
                      "void B::foo1() {}\n"
                      "void B::foo2(void) {}\n");
        ASSERT_EQUALS("", errout.str());
        const Token *functok = Token::findsimplematch(tokenizer.tokens(), "foo1 ( ) { }");
        ASSERT(functok);
        ASSERT(functok->function());
        ASSERT(functok->function()->name() == "foo1");
        functok = Token::findsimplematch(tokenizer.tokens(), "foo2 ( ) { }");
        ASSERT(functok);
        ASSERT(functok->function());
        ASSERT(functok->function()->name() == "foo2");
    }

    void symboldatabase96() { // #10126
        GET_SYMBOL_DB("struct A {\n"
                      "    int i, j;\n"
                      "};\n"
                      "std::map<int, A> m{ { 0, A{0,0} }, {0, A{0,0} } };\n");
        ASSERT_EQUALS("", errout.str());
    }

    void symboldatabase97() { // #10598 - final class
        GET_SYMBOL_DB("template<> struct A<void> final {\n"
                      "    A() {}\n"
                      "};\n");
        ASSERT(db);
        ASSERT_EQUALS(3, db->scopeList.size());

        const Token *functok = Token::findmatch(tokenizer.tokens(), "%name% (");
        ASSERT(functok);
        ASSERT(functok->function());
        ASSERT_EQUALS(functok->function()->type, Function::Type::eConstructor);
    }

    void symboldatabase98() { // #10451
        {
            GET_SYMBOL_DB("struct A { typedef struct {} B; };\n"
                          "void f() {\n"
                          "    auto g = [](A::B b) -> void { A::B b2 = b; };\n"
                          "};\n");
            ASSERT(db);
            ASSERT_EQUALS(5, db->scopeList.size());
        }
        {
            GET_SYMBOL_DB("typedef union {\n"
                          "    int i;\n"
                          "} U;\n"
                          "template <auto U::*>\n"
                          "void f();\n");
            ASSERT(db);
            ASSERT_EQUALS(2, db->scopeList.size());
        }
    }

    void symboldatabase99() { // #10864
        check("void f() { std::map<std::string, int> m; }");
        ASSERT_EQUALS("", errout.str());
    }

    void symboldatabase100() {
        {
            GET_SYMBOL_DB("namespace N {\n" // #10174
                          "    struct S {};\n"
                          "    struct T { void f(S s); };\n"
                          "    void T::f(N::S s) {}\n"
                          "}\n");
            ASSERT(db);
            ASSERT_EQUALS(1, db->functionScopes.size());
            auto it = std::find_if(db->scopeList.cbegin(), db->scopeList.cend(), [](const Scope& s) {
                return s.className == "T";
            });
            ASSERT(it != db->scopeList.end());
            const Function* function = findFunctionByName("f", &*it);
            ASSERT(function && function->token->str() == "f");
            ASSERT(function->hasBody());
        }
        {
            GET_SYMBOL_DB("namespace N {\n" // #10198
                          "    class I {};\n"
                          "    class A {\n"
                          "    public:\n"
                          "        A(I*);\n"
                          "    };\n"
                          "}\n"
                          "using N::I;\n"
                          "namespace N {\n"
                          "    A::A(I*) {}\n"
                          "}\n");
            ASSERT(db);
            ASSERT_EQUALS(1, db->functionScopes.size());
            auto it = std::find_if(db->scopeList.cbegin(), db->scopeList.cend(), [](const Scope& s) {
                return s.className == "A";
            });
            ASSERT(it != db->scopeList.end());
            const Function* function = findFunctionByName("A", &*it);
            ASSERT(function && function->token->str() == "A");
            ASSERT(function->hasBody());
        }
        {
            GET_SYMBOL_DB("namespace N {\n" // #10260
                          "    namespace O {\n"
                          "        struct B;\n"
                          "    }\n"
                          "}\n"
                          "struct I {\n"
                          "    using B = N::O::B;\n"
                          "};\n"
                          "struct A : I {\n"
                          "    void f(B*);\n"
                          "};\n"
                          "void A::f(N::O::B*) {}\n");
            ASSERT(db);
            ASSERT_EQUALS(1, db->functionScopes.size());
            auto it = std::find_if(db->scopeList.cbegin(), db->scopeList.cend(), [](const Scope& s) {
                return s.className == "A";
            });
            ASSERT(it != db->scopeList.end());
            const Function* function = findFunctionByName("f", &*it);
            ASSERT(function && function->token->str() == "f");
            ASSERT(function->hasBody());
        }
    }

    void symboldatabase101() {
        GET_SYMBOL_DB("struct A { bool b; };\n"
                      "void f(const std::vector<A>& v) {\n"
                      "    std::vector<A>::const_iterator it = b.begin();\n"
                      "    if (it->b) {}\n"
                      "}\n");
        ASSERT(db);
        const Token* it = Token::findsimplematch(tokenizer.tokens(), "it . b");
        ASSERT(it);
        ASSERT(it->tokAt(2));
        ASSERT(it->tokAt(2)->variable());
    }

    void symboldatabase102() {
        GET_SYMBOL_DB("std::string f() = delete;\n"
                      "void g() {}");
        ASSERT(db);
        ASSERT(db->scopeList.size() == 2);
        ASSERT(db->scopeList.front().type == Scope::eGlobal);
        ASSERT(db->scopeList.back().className == "g");
    }

    void symboldatabase103() {
        GET_SYMBOL_DB("void f() {\n"
                      "using lambda = decltype([]() { return true; });\n"
                      "lambda{}();\n"
                      "}\n");
        ASSERT(db != nullptr);
        ASSERT_EQUALS("", errout.str());
    }

    void symboldatabase104() {
        const bool oldDebug = settings1.debugwarnings;
        settings1.debugwarnings = true;
        {
            GET_SYMBOL_DB("struct S {\n" // #11535
                          "    void f1(char* const c);\n"
                          "    void f2(char* const c);\n"
                          "    void f3(char* const);\n"
                          "    void f4(char* c);\n"
                          "    void f5(char* c);\n"
                          "    void f6(char*);\n"
                          "};\n"
                          "void S::f1(char* c) {}\n"
                          "void S::f2(char*) {}\n"
                          "void S::f3(char* c) {}\n"
                          "void S::f4(char* const c) {}\n"
                          "void S::f5(char* const) {}\n"
                          "void S::f6(char* const c) {}\n");
            ASSERT(db != nullptr);
            ASSERT_EQUALS("", errout.str());
        }
        {
            GET_SYMBOL_DB("struct S2 {\n" // #11602
                          "    enum E {};\n"
                          "};\n"
                          "struct S1 {\n"
                          "    void f(S2::E) const;\n"
                          "};\n"
                          "void S1::f(const S2::E) const {}\n");
            ASSERT(db != nullptr);
            ASSERT_EQUALS("", errout.str());
        }
        {
            GET_SYMBOL_DB("struct S {\n"
                          "    void f(const bool b = false);\n"
                          "};\n"
                          "void S::f(const bool b) {}\n");
            ASSERT(db != nullptr);
            ASSERT_EQUALS("", errout.str());
        }
        settings1.debugwarnings = oldDebug;
    }

    void createSymbolDatabaseFindAllScopes1() {
        GET_SYMBOL_DB("void f() { union {int x; char *p;} a={0}; }");
        ASSERT(db->scopeList.size() == 3);
        ASSERT_EQUALS(Scope::eUnion, db->scopeList.back().type);
    }

    void createSymbolDatabaseFindAllScopes2() {
        GET_SYMBOL_DB("namespace ns { auto var1{0}; }\n"
                      "namespace ns { auto var2{0}; }\n");
        ASSERT(db);
        ASSERT_EQUALS(2, db->scopeList.size());
        ASSERT_EQUALS(2, db->scopeList.back().varlist.size());
        const Token* const var1 = Token::findsimplematch(tokenizer.tokens(), "var1");
        const Token* const var2 = Token::findsimplematch(tokenizer.tokens(), "var2");
        ASSERT(var1->variable());
        ASSERT(var2->variable());
    }

    void createSymbolDatabaseFindAllScopes3() {
        GET_SYMBOL_DB("namespace ns {\n"
                      "\n"
                      "namespace ns_details {\n"
                      "template <typename T, typename = void> struct has_A : std::false_type {};\n"
                      "template <typename T> struct has_A<T, typename make_void<typename T::A>::type> : std::true_type {};\n"
                      "template <typename T, bool> struct is_A_impl : public std::is_trivially_copyable<T> {};\n"
                      "template <typename T> struct is_A_impl<T, true> : public std::is_same<typename T::A, std::true_type> {};\n"
                      "}\n"
                      "\n"
                      "template <typename T> struct is_A : ns_details::is_A_impl<T, ns_details::has_A<T>::value> {};\n"
                      "template <class T, class U> struct is_A<std::pair<T, U>> : std::integral_constant<bool, is_A<T>::value && is_A<U>::value> {};\n"
                      "}\n"
                      "\n"
                      "extern \"C\" {\n"
                      "static const int foo = 8;\n"
                      "}\n");
        ASSERT(db);
        ASSERT_EQUALS(6, db->scopeList.size());
        ASSERT_EQUALS(1, db->scopeList.front().varlist.size());
        auto list = db->scopeList;
        list.pop_front();
        ASSERT_EQUALS(true, std::all_of(list.cbegin(), list.cend(), [](const Scope& scope) {
            return scope.varlist.empty();
        }));
    }

    void createSymbolDatabaseFindAllScopes4()
    {
        GET_SYMBOL_DB("struct a {\n"
                      "  void b() {\n"
                      "    std::set<int> c;\n"
                      "    a{[&] {\n"
                      "      auto d{c.lower_bound(0)};\n"
                      "      c.emplace_hint(d);\n"
                      "    }};\n"
                      "  }\n"
                      "  template <class e> a(e);\n"
                      "};\n");
        ASSERT(db);
        ASSERT_EQUALS(4, db->scopeList.size());
        const Token* const var1 = Token::findsimplematch(tokenizer.tokens(), "d");
        ASSERT(var1->variable());
    }

    void createSymbolDatabaseFindAllScopes5()
    {
        GET_SYMBOL_DB("class C {\n" // #11444
                      "public:\n"
                      "    template<typename T>\n"
                      "    class D;\n"
                      "    template<typename T>\n"
                      "    struct O : public std::false_type {};\n"
                      "};\n"
                      "template<typename T>\n"
                      "struct C::O<std::optional<T>> : public std::true_type {};\n"
                      "template<typename T>\n"
                      "class C::D {};\n"
                      "struct S {\n"
                      "    S(int i) : m(i) {}\n"
                      "    static const S IN;\n"
                      "    int m;\n"
                      "};\n"
                      "const S S::IN(1);\n");
        ASSERT(db);
        ASSERT_EQUALS(6, db->scopeList.size());
        const Token* const var = Token::findsimplematch(tokenizer.tokens(), "IN (");
        ASSERT(var && var->variable());
        ASSERT_EQUALS(var->variable()->name(), "IN");
        auto it = db->scopeList.begin();
        std::advance(it, 4);
        ASSERT_EQUALS(it->className, "S");
        ASSERT_EQUALS(var->variable()->scope(), &*it);
    }

    void createSymbolDatabaseFindAllScopes6()
    {
        GET_SYMBOL_DB("class A {\n"
                      "public:\n"
                      "  class Nested {\n"
                      "  public:\n"
                      "    virtual ~Nested() = default;\n"
                      "  };\n"
                      "};\n"
                      "class B {\n"
                      "public:\n"
                      "  class Nested {\n"
                      "  public:\n"
                      "    virtual ~Nested() = default;\n"
                      "  };\n"
                      "};\n"
                      "class C : public A, public B {\n"
                      "public:\n"
                      "  class Nested : public A::Nested, public B::Nested {};\n"
                      "};\n");
        ASSERT(db);
        ASSERT_EQUALS(6, db->typeList.size());
        auto it = db->typeList.cbegin();
        const Type& classA = *it++;
        const Type& classNA = *it++;
        const Type& classB = *it++;
        const Type& classNB = *it++;
        const Type& classC = *it++;
        const Type& classNC = *it++;
        ASSERT(classA.name() == "A" && classB.name() == "B" && classC.name() == "C");
        ASSERT(classNA.name() == "Nested" && classNB.name() == "Nested" && classNC.name() == "Nested");
        ASSERT(classA.derivedFrom.empty() && classB.derivedFrom.empty());
        ASSERT_EQUALS(classC.derivedFrom.size(), 2U);
        ASSERT_EQUALS(classC.derivedFrom[0].type, &classA);
        ASSERT_EQUALS(classC.derivedFrom[1].type, &classB);
        ASSERT(classNA.derivedFrom.empty() && classNB.derivedFrom.empty());
        ASSERT_EQUALS(classNC.derivedFrom.size(), 2U);
        ASSERT_EQUALS(classNC.derivedFrom[0].type, &classNA);
        ASSERT_EQUALS(classNC.derivedFrom[1].type, &classNB);
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

    void enum4() { // #7493
        GET_SYMBOL_DB("enum Offsets { O1, O2, O3=5, O4 };\n"
                      "enum MyEnums { E1=O1+1, E2, E3=O3+1 };");
        ASSERT(db != nullptr);

        ASSERT_EQUALS(3U, db->scopeList.size());

        // Assert that all enum values are known
        std::list<Scope>::const_iterator scope = db->scopeList.cbegin();

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
                      "int e[A + B];");
        ASSERT(db != nullptr);

        ASSERT_EQUALS(2U, db->scopeList.size());

        // Assert that all enum values are known
        std::list<Scope>::const_iterator scope = db->scopeList.cbegin();

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

        ASSERT(v->isArray());
        ASSERT_EQUALS(1U, v->dimensions().size());
        ASSERT_EQUALS(12U, v->dimension(0));
        v = db->getVariableFromVarId(2);
        ASSERT(v != nullptr);

        ASSERT(v->isArray());
        ASSERT_EQUALS(1U, v->dimensions().size());
        ASSERT_EQUALS(10U, v->dimension(0));
        v = db->getVariableFromVarId(3);
        ASSERT(v != nullptr);

        ASSERT(v->isArray());
        ASSERT_EQUALS(1U, v->dimensions().size());
        ASSERT_EQUALS(12U, v->dimension(0));
        v = db->getVariableFromVarId(4);
        ASSERT(v != nullptr);

        ASSERT(v->isArray());
        ASSERT_EQUALS(1U, v->dimensions().size());
        ASSERT_EQUALS(12U, v->dimension(0));
        v = db->getVariableFromVarId(5);
        ASSERT(v != nullptr);

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

        const Token * const functionToken = Token::findsimplematch(tokenizer.tokens(), "func");
        ASSERT(functionToken != nullptr);

        const Function *function = functionToken->function();
        ASSERT(function != nullptr);

        ASSERT(function->token->str() == "func");
        ASSERT(function->retDef && function->retDef->str() == "Enum");
        ASSERT(function->retType && function->retType->name() == "Enum");
    }

#define TEST(S) \
    v = db->getVariableFromVarId(id++); \
    ASSERT(v != nullptr); \
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
                      "char array12[sizeof(LL)];");
        ASSERT(db != nullptr);

        ASSERT(db->variableList().size() == 13); // the first one is not used
        const Variable * v;
        unsigned int id = 1;
        TEST(settings1.platform.sizeof_int);
        TEST(settings1.platform.sizeof_int);
        TEST(1);
        TEST(1);
        TEST(settings1.platform.sizeof_short);
        TEST(settings1.platform.sizeof_short);
        TEST(settings1.platform.sizeof_int);
        TEST(settings1.platform.sizeof_int);
        TEST(settings1.platform.sizeof_long);
        TEST(settings1.platform.sizeof_long);
        TEST(settings1.platform.sizeof_long_long);
        TEST(settings1.platform.sizeof_long_long);
    }

    void enum8() {
        GET_SYMBOL_DB("enum E { X0 = x, X1, X2 = 2, X3, X4 = y, X5 };\n");
        ASSERT(db != nullptr);
        const Enumerator *X0 = db->scopeList.back().findEnumerator("X0");
        ASSERT(X0);
        ASSERT(!X0->value_known);
        const Enumerator *X1 = db->scopeList.back().findEnumerator("X1");
        ASSERT(X1);
        ASSERT(!X1->value_known);
        const Enumerator *X2 = db->scopeList.back().findEnumerator("X2");
        ASSERT(X2);
        ASSERT(X2->value_known);
        ASSERT_EQUALS(X2->value, 2);
        const Enumerator *X3 = db->scopeList.back().findEnumerator("X3");
        ASSERT(X3);
        ASSERT(X3->value_known);
        ASSERT_EQUALS(X3->value, 3);
        const Enumerator *X4 = db->scopeList.back().findEnumerator("X4");
        ASSERT(X4);
        ASSERT(!X4->value_known);
        const Enumerator *X5 = db->scopeList.back().findEnumerator("X5");
        ASSERT(X5);
        ASSERT(!X5->value_known);
    }

    void enum9() {
        GET_SYMBOL_DB("const int x = 7; enum E { X0 = x, X1 };\n");
        ASSERT(db != nullptr);
        const Enumerator *X0 = db->scopeList.back().findEnumerator("X0");
        ASSERT(X0);
        ASSERT(X0->value_known);
        ASSERT_EQUALS(X0->value, 7);
        const Enumerator *X1 = db->scopeList.back().findEnumerator("X1");
        ASSERT(X1);
        ASSERT(X1->value_known);
        ASSERT_EQUALS(X1->value, 8);
    }

    void enum10() { // #11001
        GET_SYMBOL_DB_C("int b = sizeof(enum etag {X, Y});\n");
        ASSERT(db != nullptr);
        const Enumerator *X = db->scopeList.back().findEnumerator("X");
        ASSERT(X);
        ASSERT(X->value_known);
        ASSERT_EQUALS(X->value, 0);
        const Enumerator *Y = db->scopeList.back().findEnumerator("Y");
        ASSERT(Y);
        ASSERT(Y->value_known);
        ASSERT_EQUALS(Y->value, 1);
    }

    void enum11() {
        check("enum class E;\n");
        ASSERT_EQUALS("", errout.str());
    }

    void enum12() {
        GET_SYMBOL_DB_C("struct { enum E { E0 }; } t;\n"
                        "void f() {\n"
                        "    if (t.E0) {}\n"
                        "}\n");
        ASSERT(db != nullptr);
        auto it = db->scopeList.begin();
        std::advance(it, 2);
        const Enumerator* E0 = it->findEnumerator("E0");
        ASSERT(E0 && E0->value_known);
        ASSERT_EQUALS(E0->value, 0);
        const Token* const e = Token::findsimplematch(tokenizer.tokens(), "E0 )");
        ASSERT(e && e->enumerator());
        ASSERT_EQUALS(e->enumerator(), E0);
    }

    void enum13() {
        GET_SYMBOL_DB("struct S { enum E { E0, E1 }; };\n"
                      "void f(bool b) {\n"
                      "    auto e = b ? S::E0 : S::E1;\n"
                      "}\n");
        ASSERT(db != nullptr);
        auto it = db->scopeList.begin();
        std::advance(it, 2);
        const Enumerator* E1 = it->findEnumerator("E1");
        ASSERT(E1 && E1->value_known);
        ASSERT_EQUALS(E1->value, 1);
        const Token* const a = Token::findsimplematch(tokenizer.tokens(), "auto");
        ASSERT(a && a->valueType());
        ASSERT(E1->scope == a->valueType()->typeScope);
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
        std::list<Function>::const_iterator it = db->scopeList.back().functionList.cbegin();
        ASSERT((it++)->isPure());
        ASSERT((it++)->isPure());
        ASSERT(!(it++)->isPure());
        ASSERT(!(it++)->isPure());
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

    void isFunction3() {
        GET_SYMBOL_DB("std::vector<int>&& f(std::vector<int>& v) {\n"
                      "    v.push_back(1);\n"
                      "    return std::move(v);\n"
                      "}");
        ASSERT(db != nullptr);
        ASSERT_EQUALS(2, db->scopeList.size());
        const Token* ret = Token::findsimplematch(tokenizer.tokens(), "return");
        ASSERT(ret != nullptr);
        ASSERT(ret->scope() && ret->scope()->type == Scope::eFunction);
    }


    void findFunction1() {
        GET_SYMBOL_DB("int foo(int x);\n" /* 1 */
                      "void foo();\n"     /* 2 */
                      "void bar() {\n"    /* 3 */
                      "    foo();\n"      /* 4 */
                      "    foo(1);\n"     /* 5 */
                      "}");               /* 6 */
        ASSERT_EQUALS("", errout.str());
        ASSERT(db);
        const Scope * bar = db->findScopeByName("bar");
        ASSERT(bar != nullptr);
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
                      "};");
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
                      "void S::g() const &&{ }");
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
                      "    foo(0.f);\n"
                      "    foo(bar());\n"
                      "    foo(f);\n"
                      "    foo(&i);\n"
                      "    foo(ip);\n"
                      "    foo(fp);\n"
                      "    foo(cp);\n"
                      "    foo(\"\");\n"
                      "}");

        ASSERT_EQUALS("", errout.str());

        const Token* f = Token::findsimplematch(tokenizer.tokens(), "foo ( 0.f ) ;");
        ASSERT_EQUALS(true, f && f->function());
        TODO_ASSERT_EQUALS(2, 3, f->function()->tokenDef->linenr());

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( bar ( ) ) ;");
        ASSERT_EQUALS(true, f && f->function() == nullptr);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( f ) ;");
        ASSERT_EQUALS(true, f && f->function() && f->function()->tokenDef->linenr() == 2);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( & i ) ;");
        ASSERT_EQUALS(true, f && f->function() && f->function()->tokenDef->linenr() == 4);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( ip ) ;");
        ASSERT_EQUALS(true, f && f->function() && f->function()->tokenDef->linenr() == 4);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( fp ) ;");
        ASSERT_EQUALS(true, f && f->function() && f->function()->tokenDef->linenr() == 5);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( cp ) ;");
        ASSERT_EQUALS(true, f && f->function() && f->function()->tokenDef->linenr() == 5);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( \"\" ) ;");
        ASSERT_EQUALS(true, f && f->function() && f->function()->tokenDef->linenr() == 1);
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
        TODO_ASSERT(f && f->function() && f->function()->tokenDef->linenr() == 3);

        f = Token::findsimplematch(tokenizer.tokens(), "foo ( ccp ) ;");
        ASSERT_EQUALS(true, f && f->function() && f->function()->tokenDef->linenr() == 5);

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
                      "void foo1(int, char16_t a) { }\n"
                      "void foo2(int, float a) { }\n"
                      "void foo2(int, wchar_t a) { }\n"
                      "void foo3(int, float a) { }\n"
                      "void foo3(int, char a) { }\n"
                      "void func() {\n"
                      "    foo1(1, 'c');\n"
                      "    foo1(2, L'c');\n"
                      "    foo1(3, u'c');\n"
                      "    foo2(4, 'c');\n"
                      "    foo2(5, L'c');\n"
                      "    foo3(6, 'c');\n"
                      "    foo3(7, L'c');\n"
                      "}");

        ASSERT_EQUALS("", errout.str());

        const Token *f = Token::findsimplematch(tokenizer.tokens(), "foo1 ( 1");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 2);

        f = Token::findsimplematch(tokenizer.tokens(), "foo1 ( 2");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 3);

        f = Token::findsimplematch(tokenizer.tokens(), "foo1 ( 3");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 4);

        f = Token::findsimplematch(tokenizer.tokens(), "foo2 ( 4");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 6);

        f = Token::findsimplematch(tokenizer.tokens(), "foo2 ( 5");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 6);

        f = Token::findsimplematch(tokenizer.tokens(), "foo3 ( 6");
        ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 8);

        // Error: ambiguous function call
        //f = Token::findsimplematch(tokenizer.tokens(), "foo3 ( 7");
        //ASSERT_EQUALS(true, db && f && f->function() && f->function()->tokenDef->linenr() == 8);
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
        ASSERT(db);

        const Token *f = Token::findsimplematch(tokenizer.tokens(), "get ( get ( v1 ) ) ;");
        ASSERT(f);
        ASSERT(f->function());
        ASSERT_EQUALS(4, f->function()->tokenDef->linenr());

        f = Token::findsimplematch(tokenizer.tokens(), "get ( get ( v2 ) ) ;");
        ASSERT(f);
        ASSERT(f->function());
        ASSERT_EQUALS(5, f->function()->tokenDef->linenr());

        f = Token::findsimplematch(tokenizer.tokens(), "get ( get ( v3 ) ) ;");
        ASSERT(f);
        ASSERT(f->function());
        ASSERT_EQUALS(6, f->function()->tokenDef->linenr());

        f = Token::findsimplematch(tokenizer.tokens(), "get ( get ( v4 ) ) ;");
        ASSERT(f);
        ASSERT(f->function());
        ASSERT_EQUALS(7, f->function()->tokenDef->linenr());

        f = Token::findsimplematch(tokenizer.tokens(), "get ( get ( v5 ) ) ;");
        ASSERT(f);
        ASSERT(f->function());
        ASSERT_EQUALS(8, f->function()->tokenDef->linenr());

        f = Token::findsimplematch(tokenizer.tokens(), "get ( get ( v6 ) ) ;");
        ASSERT(f);
        ASSERT(f->function());
        ASSERT_EQUALS(9, f->function()->tokenDef->linenr());

        f = Token::findsimplematch(tokenizer.tokens(), "get ( get ( v7 ) ) ;");
        ASSERT(f);
        ASSERT(f->function());
        if (std::numeric_limits<char>::is_signed) {
            ASSERT_EQUALS(10, f->function()->tokenDef->linenr());
        } else {
            ASSERT_EQUALS(5, f->function()->tokenDef->linenr());
        }

        f = Token::findsimplematch(tokenizer.tokens(), "get ( get ( v8 ) ) ;");
        ASSERT(f);
        ASSERT(f->function());
        if (std::numeric_limits<char>::is_signed) {
            ASSERT_EQUALS(5, f->function()->tokenDef->linenr());
        } else {
            ASSERT_EQUALS(11, f->function()->tokenDef->linenr());
        }

        f = Token::findsimplematch(tokenizer.tokens(), "get ( get ( v9 ) ) ;");
        ASSERT(f);
        ASSERT(f->function());
        ASSERT_EQUALS(12, f->function()->tokenDef->linenr());

        f = Token::findsimplematch(tokenizer.tokens(), "get ( get ( v10 ) ) ;");
        ASSERT(f);
        ASSERT(f->function());
        ASSERT_EQUALS(13, f->function()->tokenDef->linenr());

        f = Token::findsimplematch(tokenizer.tokens(), "get ( get ( v11 ) ) ;");
        ASSERT(f);
        ASSERT(f->function());
        ASSERT_EQUALS(14, f->function()->tokenDef->linenr());

        f = Token::findsimplematch(tokenizer.tokens(), "get ( get ( v12 ) ) ;");
        ASSERT(f);
        ASSERT(f->function());
        ASSERT_EQUALS(15, f->function()->tokenDef->linenr());

        f = Token::findsimplematch(tokenizer.tokens(), "get ( get ( v13 ) ) ;");
        ASSERT(f);
        ASSERT(f->function());
        ASSERT_EQUALS(16, f->function()->tokenDef->linenr());

        f = Token::findsimplematch(tokenizer.tokens(), "get ( get ( v14 ) ) ;");
        ASSERT(f);
        ASSERT(f->function());
        ASSERT_EQUALS(17, f->function()->tokenDef->linenr());
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

    void findFunction21() { // # 8558
        GET_SYMBOL_DB("struct foo {\n"
                      "    int GetThing( ) const { return m_thing; }\n"
                      "    int* GetThing( ) { return &m_thing; }\n"
                      "};\n"
                      "\n"
                      "void f(foo *myFoo) {\n"
                      "    int* myThing = myFoo->GetThing();\n"
                      "}");

        ASSERT(db != nullptr);

        const Token *tok1 = Token::findsimplematch(tokenizer.tokens(), "myFoo . GetThing ( ) ;");

        const Function *f = tok1 && tok1->tokAt(2) ? tok1->tokAt(2)->function() : nullptr;
        ASSERT(f != nullptr);
        ASSERT_EQUALS(true, f && !f->isConst());
    }

    void findFunction22() { // # 8558
        GET_SYMBOL_DB("struct foo {\n"
                      "    int GetThing( ) const { return m_thing; }\n"
                      "    int* GetThing( ) { return &m_thing; }\n"
                      "};\n"
                      "\n"
                      "void f(const foo *myFoo) {\n"
                      "    int* myThing = myFoo->GetThing();\n"
                      "}");

        ASSERT(db != nullptr);

        const Token *tok1 = Token::findsimplematch(tokenizer.tokens(), ". GetThing ( ) ;")->next();

        const Function *f = tok1 ? tok1->function() : nullptr;
        ASSERT(f != nullptr);
        ASSERT_EQUALS(true, f && f->isConst());
    }

    void findFunction23() { // # 8558
        GET_SYMBOL_DB("struct foo {\n"
                      "    int GetThing( ) const { return m_thing; }\n"
                      "    int* GetThing( ) { return &m_thing; }\n"
                      "};\n"
                      "\n"
                      "void f(foo *myFoo) {\n"
                      "    int* myThing = ((const foo *)myFoo)->GetThing();\n"
                      "}");

        ASSERT(db != nullptr);

        const Token *tok1 = Token::findsimplematch(tokenizer.tokens(), ". GetThing ( ) ;")->next();

        const Function *f = tok1 ? tok1->function() : nullptr;
        ASSERT(f != nullptr);
        ASSERT_EQUALS(true, f && f->isConst());
    }

    void findFunction24() { // smart pointers
        GET_SYMBOL_DB("struct foo {\n"
                      "  void dostuff();\n"
                      "}\n"
                      "\n"
                      "void f(std::shared_ptr<foo> p) {\n"
                      "  p->dostuff();\n"
                      "}");
        ASSERT(db != nullptr);
        const Token *tok1 = Token::findsimplematch(tokenizer.tokens(), ". dostuff ( ) ;")->next();
        ASSERT(tok1->function());
    }

    void findFunction25() { // std::vector<std::shared_ptr<Fred>>
        GET_SYMBOL_DB("struct foo {\n"
                      "  void dostuff();\n"
                      "}\n"
                      "\n"
                      "void f1(std::vector<std::shared_ptr<foo>> v)\n"
                      "{\n"
                      "    for (auto p : v)\n"
                      "    {\n"
                      "        p->dostuff();\n"
                      "    }\n"
                      "}");
        ASSERT(db != nullptr);
        const Token *tok1 = Token::findsimplematch(tokenizer.tokens(), ". dostuff ( ) ;")->next();
        ASSERT(tok1->function());
    }

    void findFunction26() {
        GET_SYMBOL_DB("void dostuff(const int *p) {}\n"
                      "void dostuff(float) {}\n"
                      "void f(int *p) {\n"
                      "  dostuff(p);\n"
                      "}");
        ASSERT(db != nullptr);
        const Token *dostuff1 = Token::findsimplematch(tokenizer.tokens(), "dostuff ( p ) ;");
        ASSERT(dostuff1->function());
        ASSERT(dostuff1->function() && dostuff1->function()->token);
        ASSERT(dostuff1->function() && dostuff1->function()->token && dostuff1->function()->token->linenr() == 1);
    }

    void findFunction27() {
        GET_SYMBOL_DB("namespace { void a(int); }\n"
                      "void f() { a(9); }");
        const Token *a = Token::findsimplematch(tokenizer.tokens(), "a ( 9 )");
        ASSERT(a);
        ASSERT(a->function());
    }

    void findFunction28() {
        GET_SYMBOL_DB("namespace { void a(int); }\n"
                      "struct S {\n"
                      "  void foo() { a(7); }\n"
                      "  void a(int);\n"
                      "};");
        const Token *a = Token::findsimplematch(tokenizer.tokens(), "a ( 7 )");
        ASSERT(a);
        ASSERT(a->function());
        ASSERT(a->function()->token);
        ASSERT_EQUALS(4, a->function()->token->linenr());
    }

    void findFunction29() {
        GET_SYMBOL_DB("struct A {\n"
                      "    int foo() const;\n"
                      "};\n"
                      "\n"
                      "struct B {\n"
                      "    A a;\n"
                      "};\n"
                      "\n"
                      "typedef std::shared_ptr<B> BPtr;\n"
                      "\n"
                      "void bar(BPtr b) {\n"
                      "    int x = b->a.foo();\n"
                      "}");
        const Token *foo = Token::findsimplematch(tokenizer.tokens(), "foo ( ) ;");
        ASSERT(foo);
        ASSERT(foo->function());
        ASSERT(foo->function()->token);
        ASSERT_EQUALS(2, foo->function()->token->linenr());
    }

    void findFunction30() {
        GET_SYMBOL_DB("struct A;\n"
                      "void foo(std::shared_ptr<A> ptr) {\n"
                      "    int x = ptr->bar();\n"
                      "}");
        const Token *bar = Token::findsimplematch(tokenizer.tokens(), "bar ( ) ;");
        ASSERT(bar);
        ASSERT(!bar->function());
    }

    void findFunction31() {
        GET_SYMBOL_DB("void foo(bool);\n"
                      "void foo(std::string s);\n"
                      "void bar() { foo(\"123\"); }");
        const Token *foo = Token::findsimplematch(tokenizer.tokens(), "foo ( \"123\" ) ;");
        ASSERT(foo);
        ASSERT(foo->function());
        ASSERT(foo->function()->tokenDef);
        ASSERT_EQUALS(1, foo->function()->tokenDef->linenr());
    }

    void findFunction32() {
        GET_SYMBOL_DB_C("void foo(char *p);\n"
                        "void bar() { foo(\"123\"); }");
        (void)db;
        const Token *foo = Token::findsimplematch(tokenizer.tokens(), "foo ( \"123\" ) ;");
        ASSERT(foo);
        ASSERT(foo->function());
        ASSERT(foo->function()->tokenDef);
        ASSERT_EQUALS(1, foo->function()->tokenDef->linenr());
    }

    void findFunction33() {
        {
            GET_SYMBOL_DB("class Base {\n"
                          "    int i{};\n"
                          "public:\n"
                          "    void foo(...) const { bar(); }\n"
                          "    int bar() const { return i; }\n"
                          "};\n"
                          "class Derived : public Base {\n"
                          "public:\n"
                          "    void doIt() const {\n"
                          "        foo();\n"
                          "    }\n"
                          "};");
            (void)db;
            const Token *foo = Token::findsimplematch(tokenizer.tokens(), "foo ( ) ;");
            ASSERT(foo);
            ASSERT(foo->function());
            ASSERT(foo->function()->tokenDef);
            ASSERT_EQUALS(4, foo->function()->tokenDef->linenr());
        }
        {
            GET_SYMBOL_DB("class Base {\n"
                          "    int i{};\n"
                          "public:\n"
                          "    void foo(...) const { bar(); }\n"
                          "    int bar() const { return i; }\n"
                          "};\n"
                          "class Derived : public Base {\n"
                          "public:\n"
                          "    void doIt() const {\n"
                          "        foo(1);\n"
                          "    }\n"
                          "};");
            (void)db;
            const Token *foo = Token::findsimplematch(tokenizer.tokens(), "foo ( 1 ) ;");
            ASSERT(foo);
            ASSERT(foo->function());
            ASSERT(foo->function()->tokenDef);
            ASSERT_EQUALS(4, foo->function()->tokenDef->linenr());
        }
        {
            GET_SYMBOL_DB("class Base {\n"
                          "    int i{};\n"
                          "public:\n"
                          "    void foo(...) const { bar(); }\n"
                          "    int bar() const { return i; }\n"
                          "};\n"
                          "class Derived : public Base {\n"
                          "public:\n"
                          "    void doIt() const {\n"
                          "        foo(1,2);\n"
                          "    }\n"
                          "};");
            (void)db;
            const Token *foo = Token::findsimplematch(tokenizer.tokens(), "foo ( 1 , 2 ) ;");
            ASSERT(foo);
            ASSERT(foo->function());
            ASSERT(foo->function()->tokenDef);
            ASSERT_EQUALS(4, foo->function()->tokenDef->linenr());
        }
        {
            GET_SYMBOL_DB("class Base {\n"
                          "    int i{};\n"
                          "public:\n"
                          "    void foo(int, ...) const { bar(); }\n"
                          "    int bar() const { return i; }\n"
                          "};\n"
                          "class Derived : public Base {\n"
                          "public:\n"
                          "    void doIt() const {\n"
                          "        foo(1);\n"
                          "    }\n"
                          "};");
            (void)db;
            const Token *foo = Token::findsimplematch(tokenizer.tokens(), "foo ( 1 ) ;");
            ASSERT(foo);
            ASSERT(foo->function());
            ASSERT(foo->function()->tokenDef);
            ASSERT_EQUALS(4, foo->function()->tokenDef->linenr());
        }
        {
            GET_SYMBOL_DB("class Base {\n"
                          "    int i{};\n"
                          "public:\n"
                          "    void foo(int,...) const { bar(); }\n"
                          "    int bar() const { return i; }\n"
                          "};\n"
                          "class Derived : public Base {\n"
                          "public:\n"
                          "    void doIt() const {\n"
                          "        foo(1,2);\n"
                          "    }\n"
                          "};");
            (void)db;
            const Token *foo = Token::findsimplematch(tokenizer.tokens(), "foo ( 1 , 2 ) ;");
            ASSERT(foo);
            ASSERT(foo->function());
            ASSERT(foo->function()->tokenDef);
            ASSERT_EQUALS(4, foo->function()->tokenDef->linenr());
        }
        {
            GET_SYMBOL_DB("class Base {\n"
                          "    int i{};\n"
                          "public:\n"
                          "    void foo(int,...) const { bar(); }\n"
                          "    int bar() const { return i; }\n"
                          "};\n"
                          "class Derived : public Base {\n"
                          "public:\n"
                          "    void doIt() const {\n"
                          "        foo(1, 2, 3);\n"
                          "    }\n"
                          "};");
            (void)db;
            const Token *foo = Token::findsimplematch(tokenizer.tokens(), "foo ( 1 , 2 , 3 ) ;");
            ASSERT(foo);
            ASSERT(foo->function());
            ASSERT(foo->function()->tokenDef);
            ASSERT_EQUALS(4, foo->function()->tokenDef->linenr());
        }
    }

    void findFunction34() {
        GET_SYMBOL_DB("namespace cppcheck {\n"
                      "    class Platform {\n"
                      "    public:\n"
                      "        enum PlatformType { Unspecified };\n"
                      "    };\n"
                      "}\n"
                      "class ImportProject {\n"
                      "    void selectOneVsConfig(cppcheck::Platform::PlatformType);\n"
                      "};\n"
                      "class Settings : public cppcheck::Platform { };\n"
                      "void ImportProject::selectOneVsConfig(Settings::PlatformType) { }");
        (void)db;
        const Token *foo = Token::findsimplematch(tokenizer.tokens(), "selectOneVsConfig ( Settings :: PlatformType ) { }");
        ASSERT(foo);
        ASSERT(foo->function());
        ASSERT(foo->function()->tokenDef);
        ASSERT_EQUALS(8, foo->function()->tokenDef->linenr());
    }

    void findFunction35() {
        GET_SYMBOL_DB("namespace clangimport {\n"
                      "    class AstNode {\n"
                      "    public:\n"
                      "        AstNode();\n"
                      "        void createTokens();\n"
                      "    };\n"
                      "}\n"
                      "::clangimport::AstNode::AstNode() { }\n"
                      "void ::clangimport::AstNode::createTokens() { }");
        (void)db;
        const Token *foo = Token::findsimplematch(tokenizer.tokens(), "AstNode ( ) { }");
        ASSERT(foo);
        ASSERT(foo->function());
        ASSERT(foo->function()->tokenDef);
        ASSERT_EQUALS(4, foo->function()->tokenDef->linenr());
        foo = Token::findsimplematch(tokenizer.tokens(), "createTokens ( ) { }");
        ASSERT(foo);
        ASSERT(foo->function());
        ASSERT(foo->function()->tokenDef);
        ASSERT_EQUALS(5, foo->function()->tokenDef->linenr());
    }

    void findFunction36() { // #10122
        GET_SYMBOL_DB("namespace external {\n"
                      "    enum class T { };\n"
                      "}\n"
                      "namespace ns {\n"
                      "    class A {\n"
                      "    public:\n"
                      "        void f(external::T);\n"
                      "    };\n"
                      "}\n"
                      "namespace ns {\n"
                      "    void A::f(external::T link_type) { }\n"
                      "}");
        ASSERT_EQUALS("", errout.str());
        const Token *functok = Token::findsimplematch(tokenizer.tokens(), "f ( external :: T link_type )");
        ASSERT(functok);
        ASSERT(functok->function());
        ASSERT(functok->function()->name() == "f");
        ASSERT_EQUALS(7, functok->function()->tokenDef->linenr());
    }

    void findFunction37() { // #10124
        GET_SYMBOL_DB("namespace ns {\n"
                      "    class V { };\n"
                      "}\n"
                      "class A {\n"
                      "public:\n"
                      "    void f(const ns::V&);\n"
                      "};\n"
                      "using ::ns::V;\n"
                      "void A::f(const V&) { }");
        ASSERT_EQUALS("", errout.str());
        const Token *functok = Token::findsimplematch(tokenizer.tokens(), "f ( const :: ns :: V & )");
        ASSERT(functok);
        ASSERT(functok->function());
        ASSERT(functok->function()->name() == "f");
        ASSERT_EQUALS(6, functok->function()->tokenDef->linenr());
    }

    void findFunction38() { // #10125
        GET_SYMBOL_DB("namespace ns {\n"
                      "    class V { };\n"
                      "    using Var = V;\n"
                      "}\n"
                      "class A {\n"
                      "    void f(const ns::Var&);\n"
                      "};\n"
                      "using ::ns::Var;\n"
                      "void A::f(const Var&) {}");
        ASSERT_EQUALS("", errout.str());
        const Token *functok = Token::findsimplematch(tokenizer.tokens(), "f ( const :: ns :: V & )");
        ASSERT(functok);
        ASSERT(functok->function());
        ASSERT(functok->function()->name() == "f");
        ASSERT_EQUALS(6, functok->function()->tokenDef->linenr());
    }

    void findFunction39() { // #10127
        GET_SYMBOL_DB("namespace external {\n"
                      "    class V {\n"
                      "    public:\n"
                      "        using I = int;\n"
                      "    };\n"
                      "}\n"
                      "class A {\n"
                      "    void f(external::V::I);\n"
                      "};\n"
                      "using ::external::V;\n"
                      "void A::f(V::I) {}");
        ASSERT_EQUALS("", errout.str());
        const Token *functok = Token::findsimplematch(tokenizer.tokens(), "f ( int )");
        ASSERT(functok);
        ASSERT(functok->function());
        ASSERT(functok->function()->name() == "f");
        ASSERT_EQUALS(8, functok->function()->tokenDef->linenr());
    }

    void findFunction40() { // #10135
        GET_SYMBOL_DB("class E : public std::exception {\n"
                      "public:\n"
                      "    const char* what() const noexcept override;\n"
                      "};\n"
                      "const char* E::what() const noexcept {\n"
                      "    return nullptr;\n"
                      "}");
        ASSERT_EQUALS("", errout.str());
        const Token *functok = Token::findsimplematch(tokenizer.tokens(), "what ( ) const noexcept ( true ) {");
        ASSERT(functok);
        ASSERT(functok->function());
        ASSERT(functok->function()->name() == "what");
        ASSERT_EQUALS(3, functok->function()->tokenDef->linenr());
    }

    void findFunction41() { // #10202
        {
            GET_SYMBOL_DB("struct A {};\n"
                          "const int* g(const A&);\n"
                          "int* g(A&);\n"
                          "void f(A& x) {\n"
                          "    int* y = g(x);\n"
                          "    *y = 0;\n"
                          "}\n");
            ASSERT_EQUALS("", errout.str());
            const Token *functok = Token::findsimplematch(tokenizer.tokens(), "g ( x )");
            ASSERT(functok);
            ASSERT(functok->function());
            ASSERT(functok->function()->name() == "g");
            ASSERT_EQUALS(3, functok->function()->tokenDef->linenr());
        }
        {
            GET_SYMBOL_DB("struct A {};\n"
                          "const int* g(const A&);\n"
                          "int* g(A&);\n"
                          "void f(const A& x) {\n"
                          "    int* y = g(x);\n"
                          "    *y = 0;\n"
                          "}\n");
            ASSERT_EQUALS("", errout.str());
            const Token *functok = Token::findsimplematch(tokenizer.tokens(), "g ( x )");
            ASSERT(functok);
            ASSERT(functok->function());
            ASSERT(functok->function()->name() == "g");
            ASSERT_EQUALS(2, functok->function()->tokenDef->linenr());
        }
    }

    void findFunction42() {
        GET_SYMBOL_DB("void a(const std::string &, const std::string &);\n"
                      "void a(long, long);\n"
                      "void b() { a(true, false); }\n");
        ASSERT_EQUALS("", errout.str());
        const Token *functok = Token::findsimplematch(tokenizer.tokens(), "a ( true , false )");
        ASSERT(functok);
        ASSERT(functok->function());
        ASSERT(functok->function()->name() == "a");
        ASSERT_EQUALS(2, functok->function()->tokenDef->linenr());
    }

    void findFunction43() { // #10087
        {
            GET_SYMBOL_DB("struct A {};\n"
                          "const A* g(const std::string&);\n"
                          "const A& g(std::vector<A>::size_type i);\n"
                          "const A& f(std::vector<A>::size_type i) { return g(i); }\n");
            ASSERT_EQUALS("", errout.str());
            const Token *functok = Token::findsimplematch(tokenizer.tokens(), "g ( i )");
            ASSERT(functok);
            ASSERT(functok->function());
            ASSERT(functok->function()->name() == "g");
            ASSERT_EQUALS(3, functok->function()->tokenDef->linenr());
        }
        {
            GET_SYMBOL_DB("struct A {};\n"
                          "const A& g(std::vector<A>::size_type i);\n"
                          "const A* g(const std::string&);\n"
                          "const A& f(std::vector<A>::size_type i) { return g(i); }\n");
            ASSERT_EQUALS("", errout.str());
            const Token *functok = Token::findsimplematch(tokenizer.tokens(), "g ( i )");
            ASSERT(functok);
            ASSERT(functok->function());
            ASSERT(functok->function()->name() == "g");
            ASSERT_EQUALS(2, functok->function()->tokenDef->linenr());
        }
    }

    void findFunction44() { // #11182
        {
            GET_SYMBOL_DB("struct T { enum E { E0 }; };\n"
                          "struct S {\n"
                          "    void f(const void*, T::E) {}\n"
                          "    void f(const int&, T::E) {}\n"
                          "    void g() { f(nullptr, T::E0); }\n"
                          "};\n");
            ASSERT_EQUALS("", errout.str());
            const Token *functok = Token::findsimplematch(tokenizer.tokens(), "f ( nullptr");
            ASSERT(functok);
            ASSERT(functok->function());
            ASSERT(functok->function()->name() == "f");
            ASSERT_EQUALS(3, functok->function()->tokenDef->linenr());
        }
        {
            GET_SYMBOL_DB("enum E { E0 };\n"
                          "struct S {\n"
                          "    void f(int*, int) {}\n"
                          "    void f(int, int) {}\n"
                          "    void g() { f(nullptr, E0); }\n"
                          "};\n");
            ASSERT_EQUALS("", errout.str());
            const Token *functok = Token::findsimplematch(tokenizer.tokens(), "f ( nullptr");
            ASSERT(functok);
            ASSERT(functok->function());
            ASSERT(functok->function()->name() == "f");
            ASSERT_EQUALS(3, functok->function()->tokenDef->linenr());
        }
        {
            GET_SYMBOL_DB("struct T { enum E { E0 }; } t; \n" // #11559
                          "void f(const void*, T::E) {}\n"
                          "void f(const int&, T::E) {}\n"
                          "void g() {\n"
                          "    f(nullptr, t.E0);\n"
                          "}\n");
            ASSERT_EQUALS("", errout.str());
            const Token *functok = Token::findsimplematch(tokenizer.tokens(), "f ( nullptr");
            ASSERT(functok);
            ASSERT(functok->function());
            ASSERT(functok->function()->name() == "f");
            ASSERT_EQUALS(2, functok->function()->tokenDef->linenr());
        }
    }

    void findFunction45() {
        GET_SYMBOL_DB("struct S { void g(int); };\n"
                      "void f(std::vector<S>& v) {\n"
                      "    std::vector<S>::iterator it = v.begin();\n"
                      "    it->g(1);\n"
                      "}\n");
        ASSERT_EQUALS("", errout.str());
        const Token *functok = Token::findsimplematch(tokenizer.tokens(), "g ( 1");
        ASSERT(functok);
        ASSERT(functok->function());
        ASSERT(functok->function()->name() == "g");
        ASSERT_EQUALS(1, functok->function()->tokenDef->linenr());
    }

    void findFunction46() {
        GET_SYMBOL_DB("struct S {\n" // #11531
                      "    const int* g(int i, int j) const;\n"
                      "    int* g(int i, int j);\n"
                      "};\n"
                      "enum E { E0 };\n"
                      "void f(S& s, int i) {\n"
                      "    int* p = s.g(E0, i);\n"
                      "    *p = 0;\n"
                      "}\n");
        ASSERT_EQUALS("", errout.str());
        const Token *functok = Token::findsimplematch(tokenizer.tokens(), "g ( E0");
        ASSERT(functok && functok->function());
        ASSERT(functok->function()->name() == "g");
        ASSERT_EQUALS(3, functok->function()->tokenDef->linenr());
    }

    void findFunction47() { // #8592
        {
            GET_SYMBOL_DB("namespace N {\n"
                          "    void toupper(std::string& str);\n"
                          "}\n"
                          "void f(std::string s) {\n"
                          "    using namespace N;\n"
                          "    toupper(s);\n"
                          "}\n");
            ASSERT_EQUALS("", errout.str());
            const Token *functok = Token::findsimplematch(tokenizer.tokens(), "toupper ( s");
            ASSERT(functok && functok->function());
            ASSERT(functok->function()->name() == "toupper");
            ASSERT_EQUALS(2, functok->function()->tokenDef->linenr());
        }
        {
            GET_SYMBOL_DB("namespace N {\n"
                          "    void toupper(std::string& str);\n"
                          "}\n"
                          "using namespace N;\n"
                          "void f(std::string s) {\n"
                          "    toupper(s);\n"
                          "}\n");
            ASSERT_EQUALS("", errout.str());
            const Token *functok = Token::findsimplematch(tokenizer.tokens(), "toupper ( s");
            ASSERT(functok && functok->function());
            ASSERT(functok->function()->name() == "toupper");
            ASSERT_EQUALS(2, functok->function()->tokenDef->linenr());
        }
    }

    void findFunction48() {
        GET_SYMBOL_DB("struct S {\n"
                      "    S() {}\n"
                      "    std::list<S> l;\n"
                      "};\n");
        ASSERT_EQUALS("", errout.str());
        const Token* typeTok = Token::findsimplematch(tokenizer.tokens(), "S >");
        ASSERT(typeTok && typeTok->type());
        ASSERT(typeTok->type()->name() == "S");
        ASSERT_EQUALS(1, typeTok->type()->classDef->linenr());
    }

    void findFunction49() {
        GET_SYMBOL_DB("struct B {};\n"
                      "struct D : B {};\n"
                      "void f(bool = false, bool = true);\n"
                      "void f(B*, bool, bool = true);\n"
                      "void g() {\n"
                      "    D d;\n"
                      "    f(&d, true);\n"
                      "}\n");
        ASSERT_EQUALS("", errout.str());
        const Token* ftok = Token::findsimplematch(tokenizer.tokens(), "f ( &");
        ASSERT(ftok && ftok->function());
        ASSERT(ftok->function()->name() == "f");
        ASSERT_EQUALS(4, ftok->function()->tokenDef->linenr());
    }

    void findFunction50() {
        {
            GET_SYMBOL_DB("struct B { B(); void init(unsigned int value); };\n"
                          "struct D: B { D(); void init(unsigned int value); };\n"
                          "D::D() { init(0); }\n"
                          "void D::init(unsigned int value) {}\n");
            const Token* call = Token::findsimplematch(tokenizer.tokens(), "init ( 0 ) ;");
            ASSERT(call && call->function() && call->function()->functionScope);
        }

        {
            GET_SYMBOL_DB("struct B { B(); void init(unsigned int value); };\n"
                          "struct D: B { D(); void init(unsigned int value); };\n"
                          "D::D() { init(0ULL); }\n"
                          "void D::init(unsigned int value) {}\n");
            const Token* call = Token::findsimplematch(tokenizer.tokens(), "init ( 0ULL ) ;");
            ASSERT(call && call->function() && call->function()->functionScope);
        }
    }

    void findFunctionContainer() {
        {
            GET_SYMBOL_DB("void dostuff(std::vector<int> v);\n"
                          "void f(std::vector<int> v) {\n"
                          "  dostuff(v);\n"
                          "}");
            (void)db;
            const Token *dostuff = Token::findsimplematch(tokenizer.tokens(), "dostuff ( v ) ;");
            ASSERT(dostuff->function());
            ASSERT(dostuff->function() && dostuff->function()->tokenDef);
            ASSERT(dostuff->function() && dostuff->function()->tokenDef && dostuff->function()->tokenDef->linenr() == 1);
        }
        {
            GET_SYMBOL_DB("void dostuff(std::vector<int> v);\n"
                          "void dostuff(int *i);\n"
                          "void f(std::vector<char> v) {\n"
                          "  dostuff(v);\n"
                          "}");
            (void)db;
            const Token *dostuff = Token::findsimplematch(tokenizer.tokens(), "dostuff ( v ) ;");
            ASSERT(!dostuff->function());
        }
    }

    void findFunctionExternC() {
        GET_SYMBOL_DB("extern \"C\" { void foo(int); }\n"
                      "void bar() {\n"
                      "    foo(42);\n"
                      "}");
        const Token *a = Token::findsimplematch(tokenizer.tokens(), "foo ( 42 )");
        ASSERT(a);
        ASSERT(a->function());
    }

    void findFunctionGlobalScope() {
        GET_SYMBOL_DB("struct S {\n"
                      "    void foo();\n"
                      "    int x;\n"
                      "};\n"
                      "\n"
                      "int bar(int x);\n"
                      "\n"
                      "void S::foo() {\n"
                      "    x = ::bar(x);\n"
                      "}");
        const Token *bar = Token::findsimplematch(tokenizer.tokens(), "bar ( x )");
        ASSERT(bar);
        ASSERT(bar->function());
    }

    void overloadedFunction1() {
        GET_SYMBOL_DB("struct S {\n"
                      "    int operator()(int);\n"
                      "};\n"
                      "\n"
                      "void foo(S x) {\n"
                      "    x(123);\n"
                      "}");
        const Token *tok = Token::findsimplematch(tokenizer.tokens(), "x . operator() ( 123 )");
        ASSERT(tok);
        ASSERT(tok->tokAt(2)->function());
    }

    void valueTypeMatchParameter() const {
        ValueType vt_int(ValueType::Sign::SIGNED, ValueType::Type::INT, 0);
        ValueType vt_const_int(ValueType::Sign::SIGNED, ValueType::Type::INT, 0, 1);
        ASSERT_EQUALS((int)ValueType::MatchResult::SAME, (int)ValueType::matchParameter(&vt_int, &vt_int));
        ASSERT_EQUALS((int)ValueType::MatchResult::SAME, (int)ValueType::matchParameter(&vt_const_int, &vt_int));
        ASSERT_EQUALS((int)ValueType::MatchResult::SAME, (int)ValueType::matchParameter(&vt_int, &vt_const_int));

        ValueType vt_char_pointer(ValueType::Sign::SIGNED, ValueType::Type::CHAR, 1);
        ValueType vt_void_pointer(ValueType::Sign::SIGNED, ValueType::Type::VOID, 1); // compatible
        ValueType vt_int_pointer(ValueType::Sign::SIGNED, ValueType::Type::INT, 1); // not compatible
        ASSERT_EQUALS((int)ValueType::MatchResult::FALLBACK1, (int)ValueType::matchParameter(&vt_char_pointer, &vt_void_pointer));
        ASSERT_EQUALS((int)ValueType::MatchResult::NOMATCH, (int)ValueType::matchParameter(&vt_char_pointer, &vt_int_pointer));

        ValueType vt_char_pointer2(ValueType::Sign::SIGNED, ValueType::Type::CHAR, 2);
        ASSERT_EQUALS((int)ValueType::MatchResult::FALLBACK1, (int)ValueType::matchParameter(&vt_char_pointer2, &vt_void_pointer));

        ValueType vt_const_float_pointer(ValueType::Sign::UNKNOWN_SIGN, ValueType::Type::FLOAT, 1, 1);
        ValueType vt_long_long(ValueType::Sign::SIGNED, ValueType::Type::LONGLONG, 0, 0);
        ASSERT_EQUALS((int)ValueType::MatchResult::NOMATCH, (int)ValueType::matchParameter(&vt_const_float_pointer, &vt_long_long));
    }

#define FUNC(x) do { \
        const Function *x = findFunctionByName(#x, &db->scopeList.front()); \
        ASSERT_EQUALS(true, x != nullptr);                                  \
        ASSERT_EQUALS(true, x->isNoExcept()); \
} while (false)

    void noexceptFunction1() {
        GET_SYMBOL_DB("void func1() noexcept;\n"
                      "void func2() noexcept { }\n"
                      "void func3() noexcept(true);\n"
                      "void func4() noexcept(true) { }");
        ASSERT_EQUALS("", errout.str());
        ASSERT_EQUALS(true,  db != nullptr); // not null

        FUNC(func1);
        FUNC(func2);
        FUNC(func3);
        FUNC(func4);
    }

    void noexceptFunction2() {
        GET_SYMBOL_DB("template <class T> void self_assign(T& t) noexcept(noexcept(t = t)) {t = t; }");

        ASSERT_EQUALS("", errout.str());
        ASSERT_EQUALS(true,  db != nullptr); // not null

        FUNC(self_assign);
    }

#define CLASS_FUNC(x, y, z) do { \
        const Function *x = findFunctionByName(#x, y); \
        ASSERT_EQUALS(true, x != nullptr);             \
        ASSERT_EQUALS(z, x->isNoExcept()); \
} while (false)

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

        const Scope *fred = db->findScopeByName("Fred");
        ASSERT_EQUALS(true, fred != nullptr);
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
                      "};");
        ASSERT_EQUALS("", errout.str());
        ASSERT(db != nullptr); // not null
        const Scope *b = db->findScopeByName("B");
        ASSERT(b != nullptr);
        CLASS_FUNC(B, b, true);
    }

#define FUNC_THROW(x) do { \
        const Function *x = findFunctionByName(#x, &db->scopeList.front()); \
        ASSERT_EQUALS(true, x != nullptr);                                  \
        ASSERT_EQUALS(true, x->isThrow()); \
} while (false)

    void throwFunction1() {
        GET_SYMBOL_DB("void func1() throw();\n"
                      "void func2() throw() { }\n"
                      "void func3() throw(int);\n"
                      "void func4() throw(int) { }");
        ASSERT_EQUALS("", errout.str());
        ASSERT(db != nullptr); // not null

        FUNC_THROW(func1);
        FUNC_THROW(func2);
        FUNC_THROW(func3);
        FUNC_THROW(func4);
    }

#define CLASS_FUNC_THROW(x, y) do { \
        const Function *x = findFunctionByName(#x, y); \
        ASSERT_EQUALS(true, x != nullptr);             \
        ASSERT_EQUALS(true, x->isThrow()); \
} while (false)
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

        const Scope *fred = db->findScopeByName("Fred");
        ASSERT_EQUALS(true, fred != nullptr);
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

    void nothrowAttributeFunction() {
        GET_SYMBOL_DB("void func() __attribute__((nothrow));\n"
                      "void func() { }");
        ASSERT_EQUALS("", errout.str());
        ASSERT_EQUALS(true,  db != nullptr); // not null

        const Function *func = findFunctionByName("func", &db->scopeList.front());
        ASSERT_EQUALS(true, func != nullptr);
        ASSERT_EQUALS(true, func->isAttributeNothrow());
    }

    void nothrowDeclspecFunction() {
        GET_SYMBOL_DB("void __declspec(nothrow) func() { }");
        ASSERT_EQUALS("", errout.str());
        ASSERT_EQUALS(true,  db != nullptr); // not null

        const Function *func = findFunctionByName("func", &db->scopeList.front());
        ASSERT_EQUALS(true, func != nullptr);
        ASSERT_EQUALS(true, func->isAttributeNothrow());
    }

    void noreturnAttributeFunction() {
        GET_SYMBOL_DB("[[noreturn]] void func1();\n"
                      "void func1() { }\n"
                      "[[noreturn]] void func2();\n"
                      "[[noreturn]] void func3() { }\n"
                      "template <class T> [[noreturn]] void func4() { }\n"
                      "[[noreturn]] [[gnu::format(printf, 1, 2)]] void func5(const char*, ...);\n"
                      "[[gnu::format(printf, 1, 2)]] [[noreturn]] void func6(const char*, ...);\n"
                      );
        ASSERT_EQUALS("", errout.str());
        ASSERT_EQUALS(true,  db != nullptr); // not null

        const Function *func = findFunctionByName("func1", &db->scopeList.front());
        ASSERT_EQUALS(true, func != nullptr);
        ASSERT_EQUALS(true, func->isAttributeNoreturn());

        func = findFunctionByName("func2", &db->scopeList.front());
        ASSERT_EQUALS(true, func != nullptr);
        ASSERT_EQUALS(true, func->isAttributeNoreturn());

        func = findFunctionByName("func3", &db->scopeList.front());
        ASSERT_EQUALS(true, func != nullptr);
        ASSERT_EQUALS(true, func->isAttributeNoreturn());

        func = findFunctionByName("func4", &db->scopeList.front());
        ASSERT_EQUALS(true, func != nullptr);
        ASSERT_EQUALS(true, func->isAttributeNoreturn());

        func = findFunctionByName("func5", &db->scopeList.front());
        ASSERT_EQUALS(true, func != nullptr);
        ASSERT_EQUALS(true, func->isAttributeNoreturn());

        func = findFunctionByName("func6", &db->scopeList.front());
        ASSERT_EQUALS(true, func != nullptr);
        ASSERT_EQUALS(true, func->isAttributeNoreturn());

    }

    void nodiscardAttributeFunction() {
        GET_SYMBOL_DB("[[nodiscard]] int func1();\n"
                      "int func1() { }\n"
                      "[[nodiscard]] int func2();\n"
                      "[[nodiscard]] int func3() { }\n"
                      "template <class T> [[nodiscard]] int func4() { }"
                      "std::pair<bool, char> [[nodiscard]] func5();\n"
                      "[[nodiscard]] std::pair<bool, char> func6();\n"
                      );
        ASSERT_EQUALS("", errout.str());
        ASSERT_EQUALS(true,  db != nullptr); // not null

        const Function *func = findFunctionByName("func1", &db->scopeList.front());
        ASSERT_EQUALS(true, func != nullptr);
        ASSERT_EQUALS(true, func->isAttributeNodiscard());

        func = findFunctionByName("func2", &db->scopeList.front());
        ASSERT_EQUALS(true, func != nullptr);
        ASSERT_EQUALS(true, func->isAttributeNodiscard());

        func = findFunctionByName("func3", &db->scopeList.front());
        ASSERT_EQUALS(true, func != nullptr);
        ASSERT_EQUALS(true, func->isAttributeNodiscard());

        func = findFunctionByName("func4", &db->scopeList.front());
        ASSERT_EQUALS(true, func != nullptr);
        ASSERT_EQUALS(true, func->isAttributeNodiscard());

        func = findFunctionByName("func5", &db->scopeList.front());
        ASSERT_EQUALS(true, func != nullptr);
        ASSERT_EQUALS(true, func->isAttributeNodiscard());

        func = findFunctionByName("func6", &db->scopeList.front());
        ASSERT_EQUALS(true, func != nullptr);
        ASSERT_EQUALS(true, func->isAttributeNodiscard());
    }

    void varTypesIntegral() {
        GET_SYMBOL_DB("void f() { bool b; char c; unsigned char uc; short s; unsigned short us; int i; unsigned u; unsigned int ui; long l; unsigned long ul; long long ll; }");
        const Variable *b = db->getVariableFromVarId(1);
        ASSERT(b != nullptr);
        ASSERT_EQUALS("b", b->nameToken()->str());
        ASSERT_EQUALS(false, b->isFloatingType());

        const Variable *c = db->getVariableFromVarId(2);
        ASSERT(c != nullptr);
        ASSERT_EQUALS("c", c->nameToken()->str());
        ASSERT_EQUALS(false, c->isFloatingType());

        const Variable *uc = db->getVariableFromVarId(3);
        ASSERT(uc != nullptr);
        ASSERT_EQUALS("uc", uc->nameToken()->str());
        ASSERT_EQUALS(false, uc->isFloatingType());

        const Variable *s = db->getVariableFromVarId(4);
        ASSERT(s != nullptr);
        ASSERT_EQUALS("s", s->nameToken()->str());
        ASSERT_EQUALS(false, s->isFloatingType());

        const Variable *us = db->getVariableFromVarId(5);
        ASSERT(us != nullptr);
        ASSERT_EQUALS("us", us->nameToken()->str());
        ASSERT_EQUALS(false, us->isFloatingType());

        const Variable *i = db->getVariableFromVarId(6);
        ASSERT(i != nullptr);
        ASSERT_EQUALS("i", i->nameToken()->str());
        ASSERT_EQUALS(false, i->isFloatingType());

        const Variable *u = db->getVariableFromVarId(7);
        ASSERT(u != nullptr);
        ASSERT_EQUALS("u", u->nameToken()->str());
        ASSERT_EQUALS(false, u->isFloatingType());

        const Variable *ui = db->getVariableFromVarId(8);
        ASSERT(ui != nullptr);
        ASSERT_EQUALS("ui", ui->nameToken()->str());
        ASSERT_EQUALS(false, ui->isFloatingType());

        const Variable *l = db->getVariableFromVarId(9);
        ASSERT(l != nullptr);
        ASSERT_EQUALS("l", l->nameToken()->str());
        ASSERT_EQUALS(false, l->isFloatingType());

        const Variable *ul = db->getVariableFromVarId(10);
        ASSERT(ul != nullptr);
        ASSERT_EQUALS("ul", ul->nameToken()->str());
        ASSERT_EQUALS(false, ul->isFloatingType());

        const Variable *ll = db->getVariableFromVarId(11);
        ASSERT(ll != nullptr);
        ASSERT_EQUALS("ll", ll->nameToken()->str());
        ASSERT_EQUALS(false, ll->isFloatingType());
    }

    void varTypesFloating() {
        {
            GET_SYMBOL_DB("void f() { float f; double d; long double ld; }");
            const Variable *f = db->getVariableFromVarId(1);
            ASSERT(f != nullptr);
            ASSERT_EQUALS("f", f->nameToken()->str());
            ASSERT_EQUALS(true, f->isFloatingType());

            const Variable *d = db->getVariableFromVarId(2);
            ASSERT(d != nullptr);
            ASSERT_EQUALS("d", d->nameToken()->str());
            ASSERT_EQUALS(true, d->isFloatingType());

            const Variable *ld = db->getVariableFromVarId(3);
            ASSERT(ld != nullptr);
            ASSERT_EQUALS("ld", ld->nameToken()->str());
            ASSERT_EQUALS(true, ld->isFloatingType());
        }
        {
            GET_SYMBOL_DB("void f() { float * f; static const float * scf; }");
            const Variable *f = db->getVariableFromVarId(1);
            ASSERT(f != nullptr);
            ASSERT_EQUALS("f", f->nameToken()->str());
            ASSERT_EQUALS(true, f->isFloatingType());
            ASSERT_EQUALS(true, f->isArrayOrPointer());

            const Variable *scf = db->getVariableFromVarId(2);
            ASSERT(scf != nullptr);
            ASSERT_EQUALS("scf", scf->nameToken()->str());
            ASSERT_EQUALS(true, scf->isFloatingType());
            ASSERT_EQUALS(true, scf->isArrayOrPointer());
        }
        {
            GET_SYMBOL_DB("void f() { float fa[42]; }");
            const Variable *fa = db->getVariableFromVarId(1);
            ASSERT(fa != nullptr);
            ASSERT_EQUALS("fa", fa->nameToken()->str());
            ASSERT_EQUALS(true, fa->isFloatingType());
            ASSERT_EQUALS(true, fa->isArrayOrPointer());
        }
    }

    void varTypesOther() {
        GET_SYMBOL_DB("void f() { class A {} a; void *b;  }");
        const Variable *a = db->getVariableFromVarId(1);
        ASSERT(a != nullptr);
        ASSERT_EQUALS("a", a->nameToken()->str());
        ASSERT_EQUALS(false, a->isFloatingType());

        const Variable *b = db->getVariableFromVarId(2);
        ASSERT(b != nullptr);
        ASSERT_EQUALS("b", b->nameToken()->str());
        ASSERT_EQUALS(false, b->isFloatingType());
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
        std::list<Scope>::const_iterator scope = db->scopeList.cbegin();
        ASSERT_EQUALS(Scope::eGlobal, scope->type);
        ++scope;
        ASSERT_EQUALS(Scope::eFunction, scope->type);
        ++scope;
        ASSERT_EQUALS(Scope::eLambda, scope->type);
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
        std::list<Scope>::const_iterator scope = db->scopeList.cbegin();
        ASSERT_EQUALS(Scope::eGlobal, scope->type);
        ++scope;
        ASSERT_EQUALS(Scope::eFunction, scope->type);
        ++scope;
        ASSERT_EQUALS(Scope::eLambda, scope->type);
    }

    void lambda3() {
        GET_SYMBOL_DB("void func() {\n"
                      "    auto f = []() mutable {}\n"
                      "}");

        ASSERT(db && db->scopeList.size() == 3);
        std::list<Scope>::const_iterator scope = db->scopeList.cbegin();
        ASSERT_EQUALS(Scope::eGlobal, scope->type);
        ++scope;
        ASSERT_EQUALS(Scope::eFunction, scope->type);
        ++scope;
        ASSERT_EQUALS(Scope::eLambda, scope->type);
    }

    void lambda4() { // #11719
        GET_SYMBOL_DB("struct S { int* p; };\n"
                      "auto g = []() {\n"
                      "    S s;\n"
                      "    s.p = new int;\n"
                      "};\n");

        ASSERT(db && db->scopeList.size() == 3);
        std::list<Scope>::const_iterator scope = db->scopeList.cbegin();
        ASSERT_EQUALS(Scope::eGlobal, scope->type);
        ++scope;
        ASSERT_EQUALS(Scope::eStruct, scope->type);
        ++scope;
        ASSERT_EQUALS(Scope::eLambda, scope->type);
        ASSERT_EQUALS(1, scope->varlist.size());
        const Variable& s = scope->varlist.front();
        ASSERT_EQUALS(s.name(), "s");
        ASSERT(s.type());
        --scope;
        ASSERT_EQUALS(s.type()->classScope, &*scope);
    }

    void lambda5() { // #11275
        GET_SYMBOL_DB("int* f() {\n"
                      "    auto g = []<typename T>() {\n"
                      "        return true;\n"
                      "    };\n"
                      "    return nullptr;\n"
                      "}\n");

        ASSERT(db && db->scopeList.size() == 3);
        std::list<Scope>::const_iterator scope = db->scopeList.cbegin();
        ASSERT_EQUALS(Scope::eGlobal, scope->type);
        ++scope;
        ASSERT_EQUALS(Scope::eFunction, scope->type);
        ++scope;
        ASSERT_EQUALS(Scope::eLambda, scope->type);
        const Token* ret = Token::findsimplematch(tokenizer.tokens(), "return true");
        ASSERT(ret && ret->scope());
        ASSERT_EQUALS(ret->scope()->type, Scope::eLambda);
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
        {
            GET_SYMBOL_DB("class Fred {\n"
                          "    void foo(const std::string & a = \"\");\n"
                          "};\n"
                          "Fred::foo(const std::string & b) { }");

            ASSERT(db && db->scopeList.size() == 3);
            std::list<Scope>::const_iterator scope = db->scopeList.cbegin();
            ASSERT_EQUALS(Scope::eGlobal, scope->type);
            ++scope;
            ASSERT_EQUALS(Scope::eClass, scope->type);
            const Scope* class_scope = &*scope;
            ++scope;
            ASSERT(class_scope->functionList.size() == 1);
            ASSERT(class_scope->functionList.cbegin()->hasBody());
            ASSERT(class_scope->functionList.cbegin()->functionScope == &*scope);
        }
        {
            GET_SYMBOL_DB("bool f(bool (*g)(int));\n"
                          "bool f(bool (*g)(int)) { return g(0); }\n");

            ASSERT(db && db->scopeList.size() == 2);
            std::list<Scope>::const_iterator scope = db->scopeList.cbegin();
            ASSERT_EQUALS(Scope::eGlobal, scope->type);
            ASSERT(scope->functionList.size() == 1);
            ++scope;
            ASSERT_EQUALS(Scope::eFunction, scope->type);
        }
    }
#define typeOf(...) typeOf_(__FILE__, __LINE__, __VA_ARGS__)
    std::string typeOf_(const char* file, int line, const char code[], const char pattern[], const char filename[] = "test.cpp", const Settings *settings = nullptr) {
        Tokenizer tokenizer(settings ? settings : &settings2, this);
        std::istringstream istr(code);
        ASSERT_LOC(tokenizer.tokenize(istr, filename), file, line);
        const Token* tok;
        for (tok = tokenizer.list.back(); tok; tok = tok->previous())
            if (Token::simpleMatch(tok, pattern, strlen(pattern)))
                break;
        return tok->valueType() ? tok->valueType()->str() : std::string();
    }

    void valueType1() {
        // stringification
        ASSERT_EQUALS("", ValueType().str());

        Settings s;
        s.platform.int_bit = 16;
        s.platform.long_bit = 32;
        s.platform.long_long_bit = 64;

        Settings sSameSize;
        sSameSize.platform.int_bit = 32;
        sSameSize.platform.long_bit = 64;
        sSameSize.platform.long_long_bit = 64;

        // numbers
        ASSERT_EQUALS("signed int", typeOf("1;", "1", "test.c", &s));
        ASSERT_EQUALS("signed int", typeOf("(-1);", "-1", "test.c", &s));
        ASSERT_EQUALS("signed int", typeOf("32767;", "32767", "test.c", &s));
        ASSERT_EQUALS("signed int", typeOf("(-32767);", "-32767", "test.c", &s));
        ASSERT_EQUALS("signed long", typeOf("32768;", "32768", "test.c", &s));
        ASSERT_EQUALS("signed long", typeOf("(-32768);", "-32768", "test.c", &s));
        ASSERT_EQUALS("signed long", typeOf("32768l;", "32768l", "test.c", &s));
        ASSERT_EQUALS("unsigned int", typeOf("32768U;", "32768U", "test.c", &s));
        ASSERT_EQUALS("signed long long", typeOf("2147483648;", "2147483648", "test.c", &s));
        ASSERT_EQUALS("unsigned long", typeOf("2147483648u;", "2147483648u", "test.c", &s));
        ASSERT_EQUALS("signed long long", typeOf("2147483648L;", "2147483648L", "test.c", &s));
        ASSERT_EQUALS("unsigned long long", typeOf("18446744069414584320;", "18446744069414584320", "test.c", &s));
        ASSERT_EQUALS("signed int", typeOf("0xFF;", "0xFF", "test.c", &s));
        ASSERT_EQUALS("unsigned int", typeOf("0xFFU;", "0xFFU", "test.c", &s));
        ASSERT_EQUALS("unsigned int", typeOf("0xFFFF;", "0xFFFF", "test.c", &s));
        ASSERT_EQUALS("signed long", typeOf("0xFFFFFF;", "0xFFFFFF", "test.c", &s));
        ASSERT_EQUALS("unsigned long", typeOf("0xFFFFFFU;", "0xFFFFFFU", "test.c", &s));
        ASSERT_EQUALS("unsigned long", typeOf("0xFFFFFFFF;", "0xFFFFFFFF", "test.c", &s));
        ASSERT_EQUALS("signed long long", typeOf("0xFFFFFFFFFFFF;", "0xFFFFFFFFFFFF", "test.c", &s));
        ASSERT_EQUALS("unsigned long long", typeOf("0xFFFFFFFFFFFFU;", "0xFFFFFFFFFFFFU", "test.c", &s));
        ASSERT_EQUALS("unsigned long long", typeOf("0xFFFFFFFF00000000;", "0xFFFFFFFF00000000", "test.c", &s));

        ASSERT_EQUALS("signed long", typeOf("2147483648;", "2147483648", "test.c", &sSameSize));
        ASSERT_EQUALS("unsigned long", typeOf("0xc000000000000000;", "0xc000000000000000", "test.c", &sSameSize));

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
        ASSERT_EQUALS("signed int", typeOf("01;", "01"));
        ASSERT_EQUALS("unsigned int", typeOf("01U;", "01U"));
        ASSERT_EQUALS("signed long", typeOf("01L;", "01L"));
        ASSERT_EQUALS("unsigned long", typeOf("01UL;", "01UL"));
        ASSERT_EQUALS("signed long long", typeOf("01LL;", "01LL"));
        ASSERT_EQUALS("unsigned long long", typeOf("01ULL;", "01ULL"));
        ASSERT_EQUALS("signed int", typeOf("0B1;", "0B1"));
        ASSERT_EQUALS("signed int", typeOf("0b1;", "0b1"));
        ASSERT_EQUALS("unsigned int", typeOf("0b1U;", "0b1U"));
        ASSERT_EQUALS("signed long", typeOf("0b1L;", "0b1L"));
        ASSERT_EQUALS("unsigned long", typeOf("0b1UL;", "0b1UL"));
        ASSERT_EQUALS("signed long long", typeOf("0b1LL;", "0b1LL"));
        ASSERT_EQUALS("unsigned long long", typeOf("0b1ULL;", "0b1ULL"));
        ASSERT_EQUALS("float", typeOf("1.0F;", "1.0F"));
        ASSERT_EQUALS("float", typeOf("1.0f;", "1.0f"));
        ASSERT_EQUALS("double", typeOf("1.0;", "1.0"));
        ASSERT_EQUALS("double", typeOf("1E3;", "1E3"));
        ASSERT_EQUALS("double", typeOf("0x1.2p3;", "0x1.2p3"));
        ASSERT_EQUALS("long double", typeOf("1.23L;", "1.23L"));
        ASSERT_EQUALS("long double", typeOf("1.23l;", "1.23l"));

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

        // char
        ASSERT_EQUALS("char", typeOf("'a';", "'a'", "test.cpp"));
        ASSERT_EQUALS("signed int", typeOf("'a';", "'a'", "test.c"));
        ASSERT_EQUALS("wchar_t", typeOf("L'a';", "L'a'", "test.cpp"));
        ASSERT_EQUALS("wchar_t", typeOf("L'a';", "L'a'", "test.c"));
        ASSERT_EQUALS("signed int", typeOf("'aaa';", "'aaa'", "test.cpp"));
        ASSERT_EQUALS("signed int", typeOf("'aaa';", "'aaa'", "test.c"));

        // char *
        ASSERT_EQUALS("const char *", typeOf("\"hello\" + 1;", "+"));
        ASSERT_EQUALS("const char",  typeOf("\"hello\"[1];", "["));
        ASSERT_EQUALS("const char",  typeOf(";*\"hello\";", "*"));
        ASSERT_EQUALS("const wchar_t *", typeOf("L\"hello\" + 1;", "+"));

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
        ASSERT_EQUALS("signed int", typeOf("auto a(int& x, int& y) { return x + y; }", "+"));
        ASSERT_EQUALS("signed int", typeOf("auto a(int& x) { return x << 1; }", "<<"));
        ASSERT_EQUALS("signed int", typeOf("void a(int& x, int& y) { x = y; }", "=")); //Debatably this should be a signed int & but we'll stick with the current behavior for now
        ASSERT_EQUALS("signed int", typeOf("auto a(int* y) { return *y; }", "*")); //Debatably this should be a signed int & but we'll stick with the current behavior for now

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
        ASSERT_EQUALS("bool", typeOf("int x; a = (b ? false : true);", "?"));

        // Boolean operators/literals
        ASSERT_EQUALS("bool", typeOf("a > b;", ">"));
        ASSERT_EQUALS("bool", typeOf(";!b;", "!"));
        ASSERT_EQUALS("bool", typeOf("c = a && b;", "&&"));
        ASSERT_EQUALS("bool", typeOf("a = false;", "false"));
        ASSERT_EQUALS("bool", typeOf("a = true;", "true"));

        // shift => result has same type as lhs
        ASSERT_EQUALS("signed int", typeOf("int x; a = x << 1U;", "<<"));
        ASSERT_EQUALS("signed int", typeOf("int x; a = x >> 1U;", ">>"));
        ASSERT_EQUALS("",           typeOf("a = 12 >> x;", ">>", "test.cpp")); // >> might be overloaded
        ASSERT_EQUALS("signed int", typeOf("a = 12 >> x;", ">>", "test.c"));
        ASSERT_EQUALS("",           typeOf("a = 12 << x;", "<<", "test.cpp")); // << might be overloaded
        ASSERT_EQUALS("signed int", typeOf("a = 12 << x;", "<<", "test.c"));
        ASSERT_EQUALS("signed int", typeOf("a = true << 1U;", "<<"));

        // assignment => result has same type as lhs
        ASSERT_EQUALS("unsigned short", typeOf("unsigned short x; x = 3;", "="));

        // array..
        ASSERT_EQUALS("void * *", typeOf("void * x[10]; a = x + 0;", "+"));
        ASSERT_EQUALS("signed int *", typeOf("int x[10]; a = x + 1;", "+"));
        ASSERT_EQUALS("signed int",  typeOf("int x[10]; a = x[0] + 1;", "+"));
        ASSERT_EQUALS("",            typeOf("a = x[\"hello\"];", "[", "test.cpp"));
        ASSERT_EQUALS("const char",  typeOf("a = x[\"hello\"];", "[", "test.c"));
        ASSERT_EQUALS("signed int *", typeOf("int x[10]; a = &x;", "&"));
        ASSERT_EQUALS("signed int *", typeOf("int x[10]; a = &x[1];", "&"));

        // cast..
        ASSERT_EQUALS("void *", typeOf("a = (void *)0;", "("));
        ASSERT_EQUALS("char", typeOf("a = (char)32;", "("));
        ASSERT_EQUALS("signed long", typeOf("a = (long)32;", "("));
        ASSERT_EQUALS("signed long", typeOf("a = (long int)32;", "("));
        ASSERT_EQUALS("signed long long", typeOf("a = (long long)32;", "("));
        ASSERT_EQUALS("long double", typeOf("a = (long double)32;", "("));
        ASSERT_EQUALS("char", typeOf("a = static_cast<char>(32);", "("));
        ASSERT_EQUALS("", typeOf("a = (unsigned x)0;", "("));
        ASSERT_EQUALS("unsigned int", typeOf("a = unsigned(123);", "("));

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
        ASSERT_EQUALS("s", typeOf("struct s { s foo(); s(int, int); }; s s::foo() { return s(1, 2); } ", "( 1 , 2 )"));
        // Some standard template functions.. TODO library configuration
        ASSERT_EQUALS("signed int &&", typeOf("std::move(5);", "( 5 )"));

        // struct member..
        ASSERT_EQUALS("signed int", typeOf("struct AB { int a; int b; } ab; x = ab.a;", "."));
        ASSERT_EQUALS("signed int", typeOf("struct AB { int a; int b; } *ab; x = ab[1].a;", "."));

        // Overloaded operators
        ASSERT_EQUALS("Fred &", typeOf("class Fred { Fred& operator<(int); }; void f() { Fred fred; x=fred<123; }", "<"));

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

#define CHECK_LIBRARY_FUNCTION_RETURN_TYPE(type) do { \
        const char xmldata[] = "<?xml version=\"1.0\"?>\n" \
                               "<def>\n" \
                               "<function name=\"g\">\n" \
                               "<returnValue type=\"" #type "\"/>\n" \
                               "</function>\n" \
                               "</def>";              \
        const Settings sF = settingsBuilder().libraryxml(xmldata, sizeof(xmldata)).build(); \
        ASSERT_EQUALS(#type, typeOf("void f() { auto x = g(); }", "x", "test.cpp", &sF)); \
} while (false)
        // *INDENT-OFF*
        CHECK_LIBRARY_FUNCTION_RETURN_TYPE(bool);
        CHECK_LIBRARY_FUNCTION_RETURN_TYPE(signed char);
        CHECK_LIBRARY_FUNCTION_RETURN_TYPE(unsigned char);
        CHECK_LIBRARY_FUNCTION_RETURN_TYPE(signed short);
        CHECK_LIBRARY_FUNCTION_RETURN_TYPE(unsigned short);
        CHECK_LIBRARY_FUNCTION_RETURN_TYPE(signed int);
        CHECK_LIBRARY_FUNCTION_RETURN_TYPE(unsigned int);
        CHECK_LIBRARY_FUNCTION_RETURN_TYPE(signed long);
        CHECK_LIBRARY_FUNCTION_RETURN_TYPE(unsigned long);
        CHECK_LIBRARY_FUNCTION_RETURN_TYPE(signed long long);
        CHECK_LIBRARY_FUNCTION_RETURN_TYPE(unsigned long long);
        CHECK_LIBRARY_FUNCTION_RETURN_TYPE(void *);
        CHECK_LIBRARY_FUNCTION_RETURN_TYPE(void * *);
        CHECK_LIBRARY_FUNCTION_RETURN_TYPE(const void *);
        // *INDENT-ON*
#undef CHECK_LIBRARY_FUNCTION_RETURN_TYPE

        // Library types
        {
            // Char types
            Settings settings;
            const Library::PodType char8 = { 1, 'u' };
            const Library::PodType char16 = { 2, 'u' };
            const Library::PodType char32 = { 4, 'u' };
            settings.library.mPodTypes["char8_t"] = char8;
            settings.library.mPodTypes["char16_t"] = char16;
            settings.library.mPodTypes["char32_t"] = char32;
            settings.platform.sizeof_short = 2;
            settings.platform.sizeof_int = 4;

            ASSERT_EQUALS("unsigned char", typeOf("u8'a';", "u8'a'", "test.cpp", &settings));
            ASSERT_EQUALS("unsigned short", typeOf("u'a';", "u'a'", "test.cpp", &settings));
            ASSERT_EQUALS("unsigned int", typeOf("U'a';", "U'a'", "test.cpp", &settings));
            ASSERT_EQUALS("const unsigned char *", typeOf("u8\"a\";", "u8\"a\"", "test.cpp", &settings));
            ASSERT_EQUALS("const unsigned short *", typeOf("u\"a\";", "u\"a\"", "test.cpp", &settings));
            ASSERT_EQUALS("const unsigned int *", typeOf("U\"a\";", "U\"a\"", "test.cpp", &settings));
        }
        {
            // PodType
            Settings settingsWin64 = settingsBuilder().platform(cppcheck::Platform::Type::Win64).build();
            const Library::PodType u32 = { 4, 'u' };
            const Library::PodType podtype2 = { 0, 'u', Library::PodType::Type::INT };
            settingsWin64.library.mPodTypes["u32"] = u32;
            settingsWin64.library.mPodTypes["xyz::x"] = u32;
            settingsWin64.library.mPodTypes["podtype2"] = podtype2;
            ValueType vt;
            ASSERT_EQUALS(true, vt.fromLibraryType("u32", settingsWin64));
            ASSERT_EQUALS(true, vt.fromLibraryType("xyz::x", settingsWin64));
            ASSERT_EQUALS(ValueType::Type::INT, vt.type);
            ValueType vt2;
            ASSERT_EQUALS(true, vt2.fromLibraryType("podtype2", settingsWin64));
            ASSERT_EQUALS(ValueType::Type::INT, vt2.type);
            ASSERT_EQUALS("unsigned int *", typeOf(";void *data = new u32[10];", "new", "test.cpp", &settingsWin64));
            ASSERT_EQUALS("unsigned int *", typeOf(";void *data = new xyz::x[10];", "new", "test.cpp", &settingsWin64));
            ASSERT_EQUALS("unsigned int", typeOf("; x = (xyz::x)12;", "(", "test.cpp", &settingsWin64));
            ASSERT_EQUALS("unsigned int", typeOf(";u32(12);", "(", "test.cpp", &settingsWin64));
            ASSERT_EQUALS("unsigned int", typeOf("x = u32(y[i]);", "(", "test.cpp", &settingsWin64));
        }
        {
            // PlatformType
            Settings settingsUnix32 = settingsBuilder().platform(cppcheck::Platform::Type::Unix32).build();
            Library::PlatformType s32;
            s32.mType = "int";
            settingsUnix32.library.mPlatforms[settingsUnix32.platform.toString()].mPlatformTypes["s32"] = s32;
            ValueType vt;
            ASSERT_EQUALS(true, vt.fromLibraryType("s32", settingsUnix32));
            ASSERT_EQUALS(ValueType::Type::INT, vt.type);
        }
        {
            // PlatformType - wchar_t
            Settings settingsWin64 = settingsBuilder().platform(cppcheck::Platform::Type::Win64).build();
            Library::PlatformType lpctstr;
            lpctstr.mType = "wchar_t";
            settingsWin64.library.mPlatforms[settingsWin64.platform.toString()].mPlatformTypes["LPCTSTR"] = lpctstr;
            ValueType vt;
            ASSERT_EQUALS(true, vt.fromLibraryType("LPCTSTR", settingsWin64));
            ASSERT_EQUALS(ValueType::Type::WCHAR_T, vt.type);
        }
        {
            // Container
            Settings sC;
            Library::Container c;
            c.startPattern = "C";
            c.startPattern2 = "C !!::";
            sC.library.containers["C"] = c;
            ASSERT_EQUALS("container(C) *", typeOf("C*c=new C;","new","test.cpp",&sC));
            ASSERT_EQUALS("container(C) *", typeOf("x=(C*)c;","(","test.cpp",&sC));
            ASSERT_EQUALS("container(C)", typeOf("C c = C();","(","test.cpp",&sC));
        }
        {
            // Container (vector)
            Settings set;
            Library::Container vector;
            vector.startPattern = "Vector <";
            vector.startPattern2 = "Vector !!::";
            vector.type_templateArgNo = 0;
            vector.arrayLike_indexOp = true;
            vector.functions["front"] =
                Library::Container::Function{Library::Container::Action::NO_ACTION, Library::Container::Yield::ITEM};
            vector.functions["data"] =
                Library::Container::Function{Library::Container::Action::NO_ACTION, Library::Container::Yield::BUFFER};
            vector.functions["begin"] = Library::Container::Function{Library::Container::Action::NO_ACTION,
                                                                     Library::Container::Yield::START_ITERATOR};
            set.library.containers["Vector"] = vector;
            Library::Container string;
            string.startPattern = "test :: string";
            string.startPattern2 = "test :: string !!::";
            string.arrayLike_indexOp = string.stdStringLike = true;
            set.library.containers["test::string"] = string;
            ASSERT_EQUALS("signed int", typeOf("Vector<int> v; v[0]=3;", "[", "test.cpp", &set));
            ASSERT_EQUALS("container(test :: string)", typeOf("{return test::string();}", "(", "test.cpp", &set));
            ASSERT_EQUALS(
                "container(test :: string)",
                typeOf("void foo(Vector<test::string> v) { for (auto s: v) { x=s+s; } }", "s", "test.cpp", &set));
            ASSERT_EQUALS(
                "container(test :: string)",
                typeOf("void foo(Vector<test::string> v) { for (auto s: v) { x=s+s; } }", "+", "test.cpp", &set));
            ASSERT_EQUALS("container(test :: string) &",
                          typeOf("Vector<test::string> v; x = v.front();", "(", "test.cpp", &set));
            ASSERT_EQUALS("container(test :: string) *",
                          typeOf("Vector<test::string> v; x = v.data();", "(", "test.cpp", &set));
            ASSERT_EQUALS("signed int &", typeOf("Vector<int> v; x = v.front();", "(", "test.cpp", &set));
            ASSERT_EQUALS("signed int *", typeOf("Vector<int> v; x = v.data();", "(", "test.cpp", &set));
            ASSERT_EQUALS("signed int * *", typeOf("Vector<int*> v; x = v.data();", "(", "test.cpp", &set));
            ASSERT_EQUALS("iterator(Vector <)", typeOf("Vector<int> v; x = v.begin();", "(", "test.cpp", &set));
            ASSERT_EQUALS("signed int &", typeOf("Vector<int> v; x = *v.begin();", "*", "test.cpp", &set));
            ASSERT_EQUALS("container(test :: string)",
                          typeOf("void foo(){test::string s; return \"x\"+s;}", "+", "test.cpp", &set));
            ASSERT_EQUALS("container(test :: string)",
                          typeOf("void foo(){test::string s; return s+\"x\";}", "+", "test.cpp", &set));
            ASSERT_EQUALS("container(test :: string)",
                          typeOf("void foo(){test::string s; return 'x'+s;}", "+", "test.cpp", &set));
            ASSERT_EQUALS("container(test :: string)",
                          typeOf("void foo(){test::string s; return s+'x';}", "+", "test.cpp", &set));
        }

        // new
        ASSERT_EQUALS("C *", typeOf("class C {}; x = new C();", "new"));

        // auto variables
        ASSERT_EQUALS("signed int", typeOf("; auto x = 3;", "x"));
        ASSERT_EQUALS("signed int *", typeOf("; auto *p = (int *)0;", "p"));
        ASSERT_EQUALS("const signed int *", typeOf("; auto *p = (const int *)0;", "p"));
        ASSERT_EQUALS("const signed int *", typeOf("; auto *p = (constexpr int *)0;", "p"));
        ASSERT_EQUALS("const signed int *", typeOf("; const auto *p = (int *)0;", "p"));
        ASSERT_EQUALS("const signed int *", typeOf("; constexpr auto *p = (int *)0;", "p"));
        ASSERT_EQUALS("const signed int *", typeOf("; const auto *p = (const int *)0;", "p"));
        ASSERT_EQUALS("const signed int *", typeOf("; constexpr auto *p = (constexpr int *)0;", "p"));
        ASSERT_EQUALS("const signed int *", typeOf("; const constexpr auto *p = (int *)0;", "p"));
        ASSERT_EQUALS("signed int *", typeOf("; auto data = new int[100];", "data"));
        ASSERT_EQUALS("signed int", typeOf("; auto data = new X::Y; int x=1000; x=x/5;", "/")); // #7970
        ASSERT_EQUALS("signed int *", typeOf("; auto data = new (nothrow) int[100];", "data"));
        ASSERT_EQUALS("signed int *", typeOf("; auto data = new (std::nothrow) int[100];", "data"));
        ASSERT_EQUALS("const signed short", typeOf("short values[10]; void f() { for (const auto x : values); }", "x"));
        ASSERT_EQUALS("const signed short &", typeOf("short values[10]; void f() { for (const auto& x : values); }", "x"));
        ASSERT_EQUALS("signed short &", typeOf("short values[10]; void f() { for (auto& x : values); }", "x"));
        ASSERT_EQUALS("signed int * &", typeOf("int* values[10]; void f() { for (auto& p : values); }", "p"));
        ASSERT_EQUALS("signed int * const &", typeOf("int* values[10]; void f() { for (const auto& p : values); }", "p"));
        ASSERT_EQUALS("const signed int *", typeOf("int* values[10]; void f() { for (const auto* p : values); }", "p"));
        ASSERT_EQUALS("const signed int", typeOf("; const auto x = 3;", "x"));
        ASSERT_EQUALS("const signed int", typeOf("; constexpr auto x = 3;", "x"));
        ASSERT_EQUALS("const signed int", typeOf("; const constexpr auto x = 3;", "x"));
        ASSERT_EQUALS("void * const", typeOf("typedef void* HWND; const HWND x = 0;", "x"));

        // Variable declaration
        ASSERT_EQUALS("char *", typeOf("; char abc[] = \"abc\";", "["));
        ASSERT_EQUALS("", typeOf("; int x[10] = { [3]=1 };", "[ 3 ]"));

        // std::make_shared
        {
            Settings set;
            Library::SmartPointer sharedPtr;
            sharedPtr.name = "std::shared_ptr";
            set.library.smartPointers["std::shared_ptr"] = sharedPtr;
            ASSERT_EQUALS("smart-pointer(std::shared_ptr)",
                          typeOf("class C {}; x = std::make_shared<C>();", "(", "test.cpp", &set));
        }

        // return
        {
            // Container
            Settings sC;
            Library::Container c;
            c.startPattern = "C";
            c.startPattern2 = "C !!::";
            sC.library.containers["C"] = c;
            ASSERT_EQUALS("container(C)", typeOf("C f(char *p) { char data[10]; return data; }", "return", "test.cpp", &sC));
        }
        // Smart pointer
        {
            Settings set;
            Library::SmartPointer myPtr;
            myPtr.name = "MyPtr";
            set.library.smartPointers["MyPtr"] = myPtr;
            ASSERT_EQUALS("smart-pointer(MyPtr)",
                          typeOf("void f() { MyPtr<int> p; return p; }", "p ;", "test.cpp", &set));
            ASSERT_EQUALS("signed int", typeOf("void f() { MyPtr<int> p; return *p; }", "* p ;", "test.cpp", &set));
            ASSERT_EQUALS("smart-pointer(MyPtr)", typeOf("void f() {return MyPtr<int>();}", "(", "test.cpp", &set));
        }
    }

    void valueType2() {
        GET_SYMBOL_DB("int i;\n"
                      "bool b;\n"
                      "Unknown u;\n"
                      "std::string s;\n"
                      "std::vector<int> v;\n"
                      "std::map<int, void*>::const_iterator it;\n"
                      "void* p;\n"
                      "\n"
                      "void f() {\n"
                      "    func(i, b, u, s, v, it, p);\n"
                      "}");

        const Token* tok = tokenizer.tokens();

        for (int i = 0; i < 2; i++) {
            tok = Token::findsimplematch(tok, "i");
            ASSERT(tok && tok->valueType());
            ASSERT_EQUALS("signed int", tok->valueType()->str());

            tok = Token::findsimplematch(tok, "b");
            ASSERT(tok && tok->valueType());
            ASSERT_EQUALS("bool", tok->valueType()->str());

            tok = Token::findsimplematch(tok, "u");
            ASSERT(tok && !tok->valueType());

            tok = Token::findsimplematch(tok, "s");
            ASSERT(tok && tok->valueType());
            ASSERT_EQUALS("container(std :: string|wstring|u16string|u32string)", tok->valueType()->str());
            ASSERT(tok->valueType()->container && tok->valueType()->container->stdStringLike);

            tok = Token::findsimplematch(tok, "v");
            ASSERT(tok && tok->valueType());
            ASSERT_EQUALS("container(std :: vector <)", tok->valueType()->str());
            ASSERT(tok->valueType()->container && !tok->valueType()->container->stdStringLike);

            tok = Token::findsimplematch(tok, "it");
            ASSERT(tok && tok->valueType());
            ASSERT_EQUALS("iterator(std :: map|unordered_map <)", tok->valueType()->str());
        }
    }

    void valueType3() {
        {
            GET_SYMBOL_DB("void f(std::vector<std::unordered_map<int, std::unordered_set<int>>>& v, int i, int j) {\n"
                          "    auto& s = v[i][j];\n"
                          "    s.insert(0);\n"
                          "}\n");
            ASSERT_EQUALS("", errout.str());

            const Token* tok = tokenizer.tokens();
            tok = Token::findsimplematch(tok, "s .");
            ASSERT(tok && tok->valueType());
            ASSERT_EQUALS("container(std :: set|unordered_set <) &", tok->valueType()->str());
        }
        {
            GET_SYMBOL_DB("void f(std::vector<int> v) {\n"
                          "    auto it = std::find(v.begin(), v.end(), 0);\n"
                          "}\n");
            ASSERT_EQUALS("", errout.str());

            const Token* tok = tokenizer.tokens();
            tok = Token::findsimplematch(tok, "auto");
            ASSERT(tok && tok->valueType());
            ASSERT_EQUALS("iterator(std :: vector <)", tok->valueType()->str());
        }
        {
            GET_SYMBOL_DB("void f(std::vector<int>::iterator beg, std::vector<int>::iterator end) {\n"
                          "    auto it = std::find(beg, end, 0);\n"
                          "}\n");
            ASSERT_EQUALS("", errout.str());

            const Token* tok = tokenizer.tokens();
            tok = Token::findsimplematch(tok, "auto");
            ASSERT(tok && tok->valueType());
            ASSERT_EQUALS("iterator(std :: vector <)", tok->valueType()->str());
        }
        {
            GET_SYMBOL_DB("struct S {\n"
                          "    S() {}\n"
                          "    std::vector<std::shared_ptr<S>> v;\n"
                          "    void f(int i) const;\n"
                          "};\n"
                          "void S::f(int i) const {\n"
                          "    for (const auto& c : v)\n"
                          "        c->f(i);\n"
                          "}\n");
            ASSERT_EQUALS("", errout.str());

            const Token* tok = tokenizer.tokens();
            tok = Token::findsimplematch(tok, "auto");
            ASSERT(tok && tok->valueType() && tok->scope() && tok->scope()->functionOf);
            const Type* smpType = tok->valueType()->smartPointerType;
            ASSERT(smpType && smpType == tok->scope()->functionOf->definedType);
        }
        {
            GET_SYMBOL_DB("struct S { int i; };\n"
                          "void f(const std::vector<S>& v) {\n"
                          "    auto it = std::find_if(std::begin(v), std::end(v), [](const S& s) { return s.i != 0; });\n"
                          "}\n");
            ASSERT_EQUALS("", errout.str());

            const Token* tok = tokenizer.tokens();
            tok = Token::findsimplematch(tok, "auto");
            ASSERT(tok && tok->valueType());
            ASSERT_EQUALS("iterator(std :: vector <)", tok->valueType()->str());
        }
        {
            GET_SYMBOL_DB("struct S { std::vector<int> v; };\n"
                          "struct T { S s; };\n"
                          "void f(T* t) {\n"
                          "    auto it = std::find(t->s.v.begin(), t->s.v.end(), 0);\n"
                          "}\n");
            ASSERT_EQUALS("", errout.str());

            const Token* tok = tokenizer.tokens();
            tok = Token::findsimplematch(tok, "auto");
            ASSERT(tok && tok->valueType());
            ASSERT_EQUALS("iterator(std :: vector <)", tok->valueType()->str());
        }
        {
            GET_SYMBOL_DB("struct S { std::vector<int> v[1][1]; };\n"
                          "struct T { S s[1]; };\n"
                          "void f(T * t) {\n"
                          "    auto it = std::find(t->s[0].v[0][0].begin(), t->s[0].v[0][0].end(), 0);\n"
                          "}\n");
            ASSERT_EQUALS("", errout.str());

            const Token* tok = tokenizer.tokens();
            tok = Token::findsimplematch(tok, "auto");
            ASSERT(tok && tok->valueType());
            ASSERT_EQUALS("iterator(std :: vector <)", tok->valueType()->str());
        }
        {
            GET_SYMBOL_DB("std::vector<int>& g();\n"
                          "void f() {\n"
                          "    auto it = std::find(g().begin(), g().end(), 0);\n"
                          "}\n");
            ASSERT_EQUALS("", errout.str());

            const Token* tok = tokenizer.tokens();
            tok = Token::findsimplematch(tok, "auto");
            ASSERT(tok && tok->valueType());
            ASSERT_EQUALS("iterator(std :: vector <)", tok->valueType()->str());
        }
        {
            GET_SYMBOL_DB("struct T { std::set<std::string> s; };\n"
                          "struct U { std::shared_ptr<T> get(); };\n"
                          "void f(U* u) {\n"
                          "    for (const auto& str : u->get()->s) {}\n"
                          "}\n");
            ASSERT_EQUALS("", errout.str());

            const Token* tok = tokenizer.tokens();
            tok = Token::findsimplematch(tok, "auto");
            ASSERT(tok && tok->valueType());
            ASSERT_EQUALS("const container(std :: string|wstring|u16string|u32string) &", tok->valueType()->str());
        }
        {
            GET_SYMBOL_DB("void f(std::vector<int>& v) {\n"
                          "    for (auto& i : v)\n"
                          "        i = 0;\n"
                          "    for (auto&& j : v)\n"
                          "        j = 1;\n"
                          "}\n");
            ASSERT_EQUALS("", errout.str());

            const Token* tok = tokenizer.tokens();
            tok = Token::findsimplematch(tok, "auto &");
            ASSERT(tok && tok->valueType());
            ASSERT_EQUALS("signed int &", tok->valueType()->str());
            tok = Token::findsimplematch(tok, "i :");
            ASSERT(tok && tok->valueType());
            ASSERT(tok->valueType()->reference == Reference::LValue);
            tok = Token::findsimplematch(tok, "i =");
            ASSERT(tok && tok->valueType());
            ASSERT(tok->valueType()->reference == Reference::LValue);
            tok = Token::findsimplematch(tok, "auto &&");
            ASSERT(tok && tok->valueType());
            ASSERT_EQUALS("signed int &&", tok->valueType()->str());
            tok = Token::findsimplematch(tok, "j =");
            ASSERT(tok && tok->valueType());
            ASSERT(tok->valueType()->reference == Reference::RValue);
        }
        {
            GET_SYMBOL_DB("void f(std::vector<int*>& v) {\n"
                          "    for (const auto& p : v)\n"
                          "        if (p == nullptr) {}\n"
                          "}\n");
            ASSERT_EQUALS("", errout.str());

            const Token* tok = tokenizer.tokens();
            tok = Token::findsimplematch(tok, "auto");
            ASSERT(tok && tok->valueType());
            ASSERT_EQUALS("signed int * const &", tok->valueType()->str());
            tok = Token::findsimplematch(tok, "p :");
            ASSERT(tok && tok->valueType());
            ASSERT_EQUALS("signed int * const &", tok->valueType()->str());
            ASSERT(tok->variable() && tok->variable()->valueType());
            ASSERT_EQUALS("signed int * const &", tok->variable()->valueType()->str());
            tok = Token::findsimplematch(tok, "p ==");
            ASSERT(tok && tok->valueType());
            ASSERT_EQUALS("signed int * const &", tok->valueType()->str());
            ASSERT(tok->variable() && tok->variable()->valueType());
            ASSERT_EQUALS("signed int * const &", tok->variable()->valueType()->str());
        }
        {
            GET_SYMBOL_DB("auto a = 1;\n");
            ASSERT_EQUALS("", errout.str());

            const Token* tok = tokenizer.tokens();
            tok = Token::findsimplematch(tok, "auto");
            ASSERT(tok && tok->valueType());
            ASSERT_EQUALS("signed int", tok->valueType()->str());
        }
        {
            GET_SYMBOL_DB("void f(const std::string& s) {\n"
                          "    const auto* const p = s.data();\n"
                          "}\n");
            ASSERT_EQUALS("", errout.str());

            const Token* tok = tokenizer.tokens();
            tok = Token::findsimplematch(tok, "auto");
            ASSERT(tok && tok->valueType());
            ASSERT_EQUALS(ValueType::CHAR, tok->valueType()->type);
            ASSERT_EQUALS(1, tok->valueType()->constness);
            ASSERT_EQUALS(0, tok->valueType()->pointer);
            tok = Token::findsimplematch(tok, "p");
            ASSERT(tok && tok->variable() && tok->variable()->valueType());
            ASSERT_EQUALS(ValueType::CHAR, tok->variable()->valueType()->type);
            ASSERT_EQUALS(3, tok->variable()->valueType()->constness);
            ASSERT_EQUALS(1, tok->variable()->valueType()->pointer);
        }
        {
            GET_SYMBOL_DB("std::string f(const std::string& s) {\n"
                          "    auto t = s.substr(3, 7);\n"
                          "    auto r = t.substr(1, 2);\n"
                          "    return r;\n"
                          "}\n");
            ASSERT_EQUALS("", errout.str());

            const Token* tok = tokenizer.tokens();
            tok = Token::findsimplematch(tok, "auto r");
            ASSERT(tok && tok->valueType());
            ASSERT_EQUALS("container(std :: string|wstring|u16string|u32string)", tok->valueType()->str());
        }
    }

    void valueTypeThis() {
        ASSERT_EQUALS("C *", typeOf("class C { C() { *this = 0; } };", "this"));
        ASSERT_EQUALS("const C *", typeOf("class C { void foo() const; }; void C::foo() const { *this = 0; }", "this"));
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
        ASSERT(db->functionScopes[0]->function != nullptr);
        const Token *retDef = db->functionScopes[0]->function->retDef;
        ASSERT_EQUALS("func", retDef ? retDef->str() : "");
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
        ASSERT(autotok && autotok->valueType() && autotok->valueType()->pointer == 0 && autotok->valueType()->constness == 0 && autotok->valueType()->typeScope && autotok->valueType()->typeScope->definedType && autotok->valueType()->typeScope->definedType->name() == "S");
        ASSERT(autotok && autotok->type() && autotok->type()->name() == "S");

        autotok = Token::findsimplematch(autotok->next(), "auto & b");
        ASSERT(autotok && autotok->valueType() && autotok->valueType()->pointer == 0 && autotok->valueType()->constness == 0 && autotok->valueType()->typeScope && autotok->valueType()->typeScope->definedType && autotok->valueType()->typeScope->definedType->name() == "S");
        ASSERT(autotok && autotok->type() && autotok->type()->name() == "S");

        autotok = Token::findsimplematch(autotok->next(), "auto & c");
        ASSERT(autotok && autotok->valueType() && autotok->valueType()->pointer == 0 && autotok->valueType()->constness == 1 && autotok->valueType()->typeScope && autotok->valueType()->typeScope->definedType && autotok->valueType()->typeScope->definedType->name() == "S");
        ASSERT(autotok && autotok->type() && autotok->type()->name() == "S");

        autotok = Token::findsimplematch(autotok->next(), "auto * d");
        ASSERT(autotok && autotok->valueType() && autotok->valueType()->pointer == 0 && autotok->valueType()->constness == 0 && autotok->valueType()->typeScope && autotok->valueType()->typeScope->definedType && autotok->valueType()->typeScope->definedType->name() == "S");
        ASSERT(autotok && autotok->type() && autotok->type()->name() == "S");

        autotok = Token::findsimplematch(autotok->next(), "auto * e");
        ASSERT(autotok && autotok->valueType() && autotok->valueType()->pointer == 0 && autotok->valueType()->constness == 1 && autotok->valueType()->typeScope && autotok->valueType()->typeScope->definedType && autotok->valueType()->typeScope->definedType->name() == "S");
        ASSERT(autotok && autotok->type() && autotok->type()->name() == "S");

        vartok = Token::findsimplematch(tokenizer.tokens(), "a :");
        ASSERT(vartok && vartok->valueType() && vartok->valueType()->typeScope && vartok->valueType()->typeScope->definedType && vartok->valueType()->typeScope->definedType->name() == "S");
        ASSERT(vartok && vartok->variable() && !vartok->variable()->isReference() && !vartok->variable()->isPointer());
        ASSERT(vartok && vartok->variable() && vartok->variable()->type() && vartok->variable()->type()->name() == "S");

        vartok = Token::findsimplematch(vartok->next(), "b :");
        ASSERT(vartok && vartok->valueType() && vartok->valueType()->typeScope && vartok->valueType()->typeScope->definedType && vartok->valueType()->typeScope->definedType->name() == "S");
        ASSERT(vartok && vartok->variable() && vartok->variable()->isReference() && !vartok->variable()->isPointer() && !vartok->variable()->isConst());

        vartok = Token::findsimplematch(vartok->next(), "c :");
        ASSERT(vartok && vartok->valueType() && vartok->valueType()->typeScope && vartok->valueType()->typeScope->definedType && vartok->valueType()->typeScope->definedType->name() == "S");
        ASSERT(vartok && vartok->variable() && vartok->variable()->isReference() && !vartok->variable()->isPointer() && vartok->variable()->isConst());

        vartok = Token::findsimplematch(vartok->next(), "d :");
        ASSERT(vartok && vartok->valueType() && vartok->valueType()->typeScope && vartok->valueType()->typeScope->definedType && vartok->valueType()->typeScope->definedType->name() == "S");
        ASSERT(vartok && vartok->variable() && !vartok->variable()->isReference() && vartok->variable()->isPointer() && !vartok->variable()->isConst());

        vartok = Token::findsimplematch(vartok->next(), "e :");
        ASSERT(vartok && vartok->valueType() && vartok->valueType()->typeScope && vartok->valueType()->typeScope->definedType && vartok->valueType()->typeScope->definedType->name() == "S");
        ASSERT(vartok && vartok->variable() && !vartok->variable()->isReference() && vartok->variable()->isPointer() && vartok->variable()->isConst());


        vartok = Token::findsimplematch(tokenizer.tokens(), "a . i");
        ASSERT(vartok && vartok->variable() && !vartok->variable()->isPointer() && !vartok->variable()->isReference() && vartok->variable()->type() && vartok->variable()->type()->name() == "S");

        vartok = Token::findsimplematch(vartok, "i");
        ASSERT(vartok && vartok->variable() && vartok->variable()->typeStartToken()->str() == "int");

        vartok = Token::findsimplematch(vartok->next(), "b . i");
        ASSERT(vartok && vartok->variable() && !vartok->variable()->isPointer() && vartok->variable()->isReference() && vartok->variable()->type() && vartok->variable()->type()->name() == "S");

        vartok = Token::findsimplematch(vartok->next(), "i");
        ASSERT(vartok && vartok->variable() && vartok->variable()->typeStartToken()->str() == "int");

        vartok = Token::findsimplematch(vartok->next(), "c . i");
        ASSERT(vartok && vartok->variable() && !vartok->variable()->isPointer() && vartok->variable()->isReference() && vartok->variable()->type() && vartok->variable()->type()->name() == "S");

        vartok = Token::findsimplematch(vartok->next(), "i");
        ASSERT(vartok && vartok->variable() && vartok->variable()->typeStartToken()->str() == "int");

        vartok = Token::findsimplematch(vartok->next(), "d . i");
        ASSERT(vartok && vartok->variable() && vartok->variable()->isPointer() && !vartok->variable()->isReference() && vartok->variable()->type() && vartok->variable()->type()->name() == "S");

        vartok = Token::findsimplematch(vartok->next(), "i");
        ASSERT(vartok && vartok->variable() && vartok->variable()->typeStartToken()->str() == "int");

        vartok = Token::findsimplematch(vartok->next(), "e . i");
        ASSERT(vartok && vartok->variable() && vartok->variable()->isPointer() && !vartok->variable()->isReference() && vartok->variable()->type() && vartok->variable()->type()->name() == "S");

        vartok = Token::findsimplematch(vartok->next(), "i");
        ASSERT(vartok && vartok->variable() && vartok->variable()->typeStartToken()->str() == "int");

        vartok = Token::findsimplematch(vartok->next(), "i");
        ASSERT(vartok && vartok->variable() && vartok->variable()->typeStartToken()->str() == "int");
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
        ASSERT(autotok);
        ASSERT(autotok->valueType());
        ASSERT_EQUALS(0, autotok->valueType()->constness);
        ASSERT_EQUALS(0, autotok->valueType()->pointer);
        ASSERT_EQUALS(ValueType::SIGNED, autotok->valueType()->sign);
        ASSERT_EQUALS(ValueType::INT, autotok->valueType()->type);
        vartok = Token::findsimplematch(autotok, "v1 =");
        ASSERT(autotok);
        ASSERT(autotok->valueType());
        ASSERT_EQUALS(0, vartok->valueType()->constness);
        ASSERT_EQUALS(0, vartok->valueType()->pointer);
        ASSERT_EQUALS(ValueType::SIGNED, vartok->valueType()->sign);
        ASSERT_EQUALS(ValueType::INT, vartok->valueType()->type);

        // auto = int, v2 = int
        autotok = Token::findsimplematch(autotok, "auto v2");
        ASSERT(autotok);
        ASSERT(autotok->valueType());
        ASSERT_EQUALS(0, autotok->valueType()->constness);
        ASSERT_EQUALS(0, autotok->valueType()->pointer);
        ASSERT_EQUALS(ValueType::SIGNED, autotok->valueType()->sign);
        ASSERT_EQUALS(ValueType::INT, autotok->valueType()->type);
        vartok = Token::findsimplematch(autotok, "v2 =");
        ASSERT(autotok);
        ASSERT(autotok->valueType());
        ASSERT_EQUALS(0, vartok->valueType()->constness);
        ASSERT_EQUALS(0, vartok->valueType()->pointer);
        ASSERT_EQUALS(ValueType::SIGNED, vartok->valueType()->sign);
        ASSERT_EQUALS(ValueType::INT, vartok->valueType()->type);

        // auto = const int *, v3 = const int * (const int[10])
        autotok = Token::findsimplematch(autotok, "auto v3");
        ASSERT(autotok);
        ASSERT(autotok->valueType());
        ASSERT_EQUALS(1, autotok->valueType()->constness);
        ASSERT_EQUALS(1, autotok->valueType()->pointer);
        ASSERT_EQUALS(ValueType::SIGNED, autotok->valueType()->sign);
        ASSERT_EQUALS(ValueType::INT, autotok->valueType()->type);
        vartok = Token::findsimplematch(autotok, "v3 =");
        ASSERT(autotok);
        ASSERT(autotok->valueType());
        ASSERT_EQUALS(1, vartok->valueType()->constness);
        ASSERT_EQUALS(1, vartok->valueType()->pointer);
        ASSERT_EQUALS(ValueType::SIGNED, vartok->valueType()->sign);
        ASSERT_EQUALS(ValueType::INT, vartok->valueType()->type);

        // auto = int *, v4 = const int * (const int[10])
        autotok = Token::findsimplematch(autotok, "auto v4");
        ASSERT(autotok);
        ASSERT(autotok->valueType());
        ASSERT_EQUALS(3, autotok->valueType()->constness);
        ASSERT_EQUALS(1, autotok->valueType()->pointer);
        ASSERT_EQUALS(ValueType::SIGNED, autotok->valueType()->sign);
        ASSERT_EQUALS(ValueType::INT, autotok->valueType()->type);
        vartok = Token::findsimplematch(autotok, "v4 =");
        ASSERT(autotok);
        ASSERT(autotok->valueType());
        ASSERT_EQUALS(3, vartok->valueType()->constness);
        ASSERT_EQUALS(1, vartok->valueType()->pointer);
        ASSERT_EQUALS(ValueType::SIGNED, vartok->valueType()->sign);
        ASSERT_EQUALS(ValueType::INT, vartok->valueType()->type);

        // auto = int, v5 = const int * (const int[10])
        autotok = Token::findsimplematch(autotok, "auto * v5");
        ASSERT(autotok);
        ASSERT(autotok->valueType());
        TODO_ASSERT_EQUALS(0, 1, autotok->valueType()->constness);
        ASSERT_EQUALS(0, autotok->valueType()->pointer);
        ASSERT_EQUALS(ValueType::SIGNED, autotok->valueType()->sign);
        ASSERT_EQUALS(ValueType::INT, autotok->valueType()->type);
        vartok = Token::findsimplematch(autotok, "v5 =");
        ASSERT(autotok);
        ASSERT(autotok->valueType());
        ASSERT_EQUALS(1, vartok->valueType()->constness);
        ASSERT_EQUALS(1, vartok->valueType()->pointer);
        ASSERT_EQUALS(ValueType::SIGNED, vartok->valueType()->sign);
        ASSERT_EQUALS(ValueType::INT, vartok->valueType()->type);

        // auto = int, v6 = int
        autotok = Token::findsimplematch(autotok, "auto v6");
        ASSERT(autotok);
        ASSERT(autotok->valueType());
        ASSERT_EQUALS(0, autotok->valueType()->constness);
        ASSERT_EQUALS(0, autotok->valueType()->pointer);
        ASSERT_EQUALS(ValueType::SIGNED, autotok->valueType()->sign);
        ASSERT_EQUALS(ValueType::INT, autotok->valueType()->type);
        vartok = Token::findsimplematch(autotok, "v6 =");
        ASSERT(autotok);
        ASSERT(autotok->valueType());
        ASSERT_EQUALS(0, vartok->valueType()->constness);
        ASSERT_EQUALS(0, vartok->valueType()->pointer);
        ASSERT_EQUALS(ValueType::SIGNED, vartok->valueType()->sign);
        ASSERT_EQUALS(ValueType::INT, vartok->valueType()->type);

        // auto = int, v7 = int
        autotok = Token::findsimplematch(autotok, "auto v7");
        ASSERT(autotok);
        ASSERT(autotok->valueType());
        ASSERT_EQUALS(0, autotok->valueType()->constness);
        ASSERT_EQUALS(0, autotok->valueType()->pointer);
        ASSERT_EQUALS(ValueType::SIGNED, autotok->valueType()->sign);
        ASSERT_EQUALS(ValueType::INT, autotok->valueType()->type);
        vartok = Token::findsimplematch(autotok, "v7 =");
        ASSERT(autotok);
        ASSERT(autotok->valueType());
        ASSERT_EQUALS(0, vartok->valueType()->constness);
        ASSERT_EQUALS(0, vartok->valueType()->pointer);
        ASSERT_EQUALS(ValueType::SIGNED, vartok->valueType()->sign);
        ASSERT_EQUALS(ValueType::INT, vartok->valueType()->type);

        // auto = const int *, v8 = const int * (const int[10])
        autotok = Token::findsimplematch(autotok, "auto v8");
        ASSERT(autotok);
        ASSERT(autotok->valueType());
        ASSERT_EQUALS(1, autotok->valueType()->constness);
        ASSERT_EQUALS(1, autotok->valueType()->pointer);
        ASSERT_EQUALS(ValueType::SIGNED, autotok->valueType()->sign);
        ASSERT_EQUALS(ValueType::INT, autotok->valueType()->type);
        vartok = Token::findsimplematch(autotok, "v8 =");
        ASSERT(autotok);
        ASSERT(autotok->valueType());
        ASSERT_EQUALS(1, vartok->valueType()->constness);
        ASSERT_EQUALS(1, vartok->valueType()->pointer);
        ASSERT_EQUALS(ValueType::SIGNED, vartok->valueType()->sign);
        ASSERT_EQUALS(ValueType::INT, vartok->valueType()->type);

        // auto = int *, v9 = const int * (const int[10])
        autotok = Token::findsimplematch(autotok, "auto v9");
        ASSERT(autotok);
        ASSERT(autotok->valueType());
        ASSERT_EQUALS(3, autotok->valueType()->constness);
        ASSERT_EQUALS(1, autotok->valueType()->pointer);
        ASSERT_EQUALS(ValueType::SIGNED, autotok->valueType()->sign);
        ASSERT_EQUALS(ValueType::INT, autotok->valueType()->type);
        vartok = Token::findsimplematch(autotok, "v9 =");
        ASSERT(autotok);
        ASSERT(autotok->valueType());
        ASSERT_EQUALS(3, vartok->valueType()->constness);
        ASSERT_EQUALS(1, vartok->valueType()->pointer);
        ASSERT_EQUALS(ValueType::SIGNED, vartok->valueType()->sign);
        ASSERT_EQUALS(ValueType::INT, vartok->valueType()->type);

        // auto = int, v10 = const int * (const int[10])
        autotok = Token::findsimplematch(autotok, "auto * v10");
        ASSERT(autotok);
        ASSERT(autotok->valueType());
        TODO_ASSERT_EQUALS(0, 1, autotok->valueType()->constness);
        ASSERT_EQUALS(0, autotok->valueType()->pointer);
        ASSERT_EQUALS(ValueType::SIGNED, autotok->valueType()->sign);
        ASSERT_EQUALS(ValueType::INT, autotok->valueType()->type);
        vartok = Token::findsimplematch(autotok, "v10 =");
        ASSERT(autotok);
        ASSERT(autotok->valueType());
        ASSERT_EQUALS(1, vartok->valueType()->constness);
        ASSERT_EQUALS(1, vartok->valueType()->pointer);
        ASSERT_EQUALS(ValueType::SIGNED, vartok->valueType()->sign);
        ASSERT_EQUALS(ValueType::INT, vartok->valueType()->type);

        // auto = int, v11 = int
        autotok = Token::findsimplematch(autotok, "auto v11");
        ASSERT(autotok);
        ASSERT(autotok->valueType());
        ASSERT_EQUALS(0, autotok->valueType()->constness);
        ASSERT_EQUALS(0, autotok->valueType()->pointer);
        ASSERT_EQUALS(ValueType::SIGNED, autotok->valueType()->sign);
        ASSERT_EQUALS(ValueType::INT, autotok->valueType()->type);

        vartok = autotok->next();
        ASSERT(autotok);
        ASSERT(autotok->valueType());
        ASSERT_EQUALS(0, vartok->valueType()->constness);
        ASSERT_EQUALS(0, vartok->valueType()->pointer);
        ASSERT_EQUALS(ValueType::SIGNED, vartok->valueType()->sign);
        ASSERT_EQUALS(ValueType::INT, vartok->valueType()->type);
    }

    void auto8() {
        GET_SYMBOL_DB("std::vector<int> vec;\n"
                      "void foo() {\n"
                      "    for (auto it = vec.begin(); it != vec.end(); ++it) { }\n"
                      "}");
        const Token *autotok = Token::findsimplematch(tokenizer.tokens(), "auto it");

        ASSERT(autotok);
        ASSERT(autotok->valueType());
        ASSERT_EQUALS(0, autotok->valueType()->constness);
        ASSERT_EQUALS(0, autotok->valueType()->pointer);
        ASSERT_EQUALS(ValueType::UNKNOWN_SIGN, autotok->valueType()->sign);
        ASSERT_EQUALS(ValueType::ITERATOR, autotok->valueType()->type);

        vartok = Token::findsimplematch(autotok, "it =");
        ASSERT(vartok);
        ASSERT(vartok->valueType());
        ASSERT_EQUALS(0, vartok->valueType()->constness);
        ASSERT_EQUALS(0, vartok->valueType()->pointer);
        ASSERT_EQUALS(ValueType::UNKNOWN_SIGN, vartok->valueType()->sign);
        ASSERT_EQUALS(ValueType::ITERATOR, vartok->valueType()->type);
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
    }

    void auto10() { // #8020
        GET_SYMBOL_DB("void f() {\n"
                      "    std::vector<int> ints(4);\n"
                      "    auto iter = ints.begin() + (ints.size() - 1);\n"
                      "}");
        const Token *autotok = Token::findsimplematch(tokenizer.tokens(), "auto iter");

        ASSERT(autotok);
        ASSERT(autotok->valueType());
        ASSERT_EQUALS(0, autotok->valueType()->constness);
        ASSERT_EQUALS(0, autotok->valueType()->pointer);
        ASSERT_EQUALS(ValueType::UNKNOWN_SIGN, autotok->valueType()->sign);
        ASSERT_EQUALS(ValueType::ITERATOR, autotok->valueType()->type);
    }

    void auto11() {
        GET_SYMBOL_DB("void f() {\n"
                      "  const auto v1 = 3;\n"
                      "  const auto *v2 = 0;\n"
                      "}");

        const Token *v1tok = Token::findsimplematch(tokenizer.tokens(), "v1");
        ASSERT(v1tok && v1tok->variable() && v1tok->variable()->isConst());

        const Token *v2tok = Token::findsimplematch(tokenizer.tokens(), "v2");
        ASSERT(v2tok && v2tok->variable() && !v2tok->variable()->isConst());
    }

    void auto12() {
        GET_SYMBOL_DB("void f(const std::string &x) {\n"
                      "  auto y = x;\n"
                      "  if (y.empty()) {}\n"
                      "}");

        const Token *tok;

        tok = Token::findsimplematch(tokenizer.tokens(), "y =");
        ASSERT(tok && tok->valueType() && tok->valueType()->container);

        tok = Token::findsimplematch(tokenizer.tokens(), "y .");
        ASSERT(tok && tok->valueType() && tok->valueType()->container);
    }

    void auto13() {
        GET_SYMBOL_DB("uint8_t *get();\n"
                      "\n"
                      "uint8_t *test()\n"
                      "{\n"
                      "    auto *next = get();\n"
                      "    return next;\n"
                      "}");

        const Token *tok;

        tok = Token::findsimplematch(tokenizer.tokens(), "return")->next();
        ASSERT(tok);
        ASSERT(tok->valueType());
        ASSERT(tok->valueType()->pointer);
        ASSERT(tok->variable()->valueType());
        ASSERT(tok->variable()->valueType()->pointer);
    }

    void auto14() { // #9892 - crash in Token::declType
        GET_SYMBOL_DB("static void foo() {\n"
                      "    auto combo = widget->combo = new Combo{};\n"
                      "    combo->addItem();\n"
                      "}");

        const Token *tok;

        tok = Token::findsimplematch(tokenizer.tokens(), "combo =");
        ASSERT(tok && !tok->valueType());
    }

    void auto15() {
        GET_SYMBOL_DB("auto var1{3};\n"
                      "auto var2{4.0};");
        ASSERT_EQUALS(3, db->variableList().size());
        const Variable *var1 = db->variableList()[1];
        ASSERT(var1->valueType());
        ASSERT_EQUALS(ValueType::Type::INT, var1->valueType()->type);
        const Variable *var2 = db->variableList()[2];
        ASSERT(var2->valueType());
        ASSERT_EQUALS(ValueType::Type::DOUBLE, var2->valueType()->type);
    }

    void auto16() {
        GET_SYMBOL_DB("void foo(std::map<std::string, bool> x) {\n"
                      "    for (const auto& i: x) {}\n"
                      "}\n");
        ASSERT_EQUALS(3, db->variableList().size());
        const Variable *i = db->variableList().back();
        ASSERT(i->valueType());
        ASSERT_EQUALS(ValueType::Type::RECORD, i->valueType()->type);
    }

    void auto17() { // #11163 don't hang
        GET_SYMBOL_DB("void f() {\n"
                      "    std::shared_ptr<int> s1;\n"
                      "    auto s2 = std::shared_ptr(s1);\n"
                      "}\n"
                      "void g() {\n"
                      "    std::shared_ptr<int> s;\n"
                      "    auto w = std::weak_ptr(s);\n"
                      "}\n");
        ASSERT_EQUALS(5, db->variableList().size());
    }

    void auto18() {
        GET_SYMBOL_DB("void f(const int* p) {\n"
                      "    const int* const& r = p;\n"
                      "    const auto& s = p;\n"
                      "}\n");
        ASSERT_EQUALS(4, db->variableList().size());

        const Variable* r = db->variableList()[2];
        ASSERT(r->isReference());
        ASSERT(r->isConst());
        ASSERT(r->isPointer());
        const Token* varTok = Token::findsimplematch(tokenizer.tokens(), "r");
        ASSERT(varTok && varTok->valueType());
        ASSERT_EQUALS(varTok->valueType()->constness, 3);
        ASSERT_EQUALS(varTok->valueType()->pointer, 1);
        ASSERT(varTok->valueType()->reference == Reference::LValue);

        const Variable* s = db->variableList()[3];
        ASSERT(s->isReference());
        ASSERT(s->isConst());
        ASSERT(s->isPointer());
        const Token* autoTok = Token::findsimplematch(tokenizer.tokens(), "auto");
        ASSERT(autoTok && autoTok->valueType());
        ASSERT_EQUALS(autoTok->valueType()->constness, 3);
        ASSERT_EQUALS(autoTok->valueType()->pointer, 1);
    }

    void auto19() { // #11517
        {
            GET_SYMBOL_DB("void f(const std::vector<void*>& v) {\n"
                          "    for (const auto* h : v)\n"
                          "        if (h) {}\n"
                          "}\n");
            ASSERT_EQUALS(3, db->variableList().size());

            const Variable* h = db->variableList()[2];
            ASSERT(h->isPointer());
            ASSERT(!h->isConst());
            const Token* varTok = Token::findsimplematch(tokenizer.tokens(), "h )");
            ASSERT(varTok && varTok->valueType());
            ASSERT_EQUALS(varTok->valueType()->constness, 1);
            ASSERT_EQUALS(varTok->valueType()->pointer, 1);
        }
        {
            GET_SYMBOL_DB("struct B { virtual void f() {} };\n"
                          "struct D : B {};\n"
                          "void g(const std::vector<B*>& v) {\n"
                          "    for (auto* b : v)\n"
                          "        if (auto d = dynamic_cast<D*>(b))\n"
                          "            d->f();\n"
                          "}\n");
            ASSERT_EQUALS(4, db->variableList().size());

            const Variable* b = db->variableList()[2];
            ASSERT(b->isPointer());
            ASSERT(!b->isConst());
            const Token* varTok = Token::findsimplematch(tokenizer.tokens(), "b )");
            ASSERT(varTok && varTok->valueType());
            ASSERT_EQUALS(varTok->valueType()->constness, 0);
            ASSERT_EQUALS(varTok->valueType()->pointer, 1);
        }
    }

    void auto20() {
        GET_SYMBOL_DB("enum A { A0 };\n"
                      "enum B { B0 };\n"
                      "const int& g(A a);\n"
                      "const int& g(B b);\n"
                      "void f() {\n"
                      "    const auto& r = g(B::B0);\n"
                      "}\n");
        const Token* a = Token::findsimplematch(tokenizer.tokens(), "auto");
        ASSERT(a && a->valueType());
        ASSERT_EQUALS(a->valueType()->type, ValueType::INT);
        const Token* g = Token::findsimplematch(a, "g ( B ::");
        ASSERT(g && g->function());
        ASSERT_EQUALS(g->function()->tokenDef->linenr(), 4);
    }

    void auto21() {
        GET_SYMBOL_DB("int f(bool b) {\n"
                      "    std::vector<int> v1(1), v2(2);\n"
                      "    auto& v = b ? v1 : v2;\n"
                      "    v.push_back(1);\n"
                      "    return v.size();\n"
                      "}\n");
        ASSERT_EQUALS("", errout.str());
        const Token* a = Token::findsimplematch(tokenizer.tokens(), "auto");
        ASSERT(a && a->valueType());
        ASSERT_EQUALS(a->valueType()->type, ValueType::CONTAINER);
        const Token* v = Token::findsimplematch(a, "v . size");
        ASSERT(v && v->valueType());
        ASSERT_EQUALS(v->valueType()->type, ValueType::CONTAINER);
        ASSERT(v->variable() && v->variable()->isReference());
    }

    void auto22() {
        GET_SYMBOL_DB("void f(std::vector<std::string>& v, bool b) {\n"
                      "    auto& s = b ? v[0] : v[1];\n"
                      "    s += \"abc\";\n"
                      "}\n");
        ASSERT_EQUALS("", errout.str());
        const Token* a = Token::findsimplematch(tokenizer.tokens(), "auto");
        ASSERT(a && a->valueType());
        ASSERT_EQUALS(a->valueType()->type, ValueType::CONTAINER);
        const Token* s = Token::findsimplematch(a, "s +=");
        ASSERT(s && s->valueType());
        ASSERT_EQUALS(s->valueType()->type, ValueType::CONTAINER);
        ASSERT(s->valueType()->reference == Reference::LValue);
        ASSERT(s->variable() && s->variable()->isReference());
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
        ASSERT(f && f->function() && f->function()->tokenDef->linenr() == 2);

        f = Token::findsimplematch(tokenizer.tokens(), "Fred ( float");
        ASSERT(f && f->function() && f->function()->tokenDef->linenr() == 3);
    }

    void incomplete_type() {
        GET_SYMBOL_DB("template<class _Ty,\n"
                      "  class _Alloc = std::allocator<_Ty>>\n"
                      "  class SLSurfaceLayerData\n"
                      "  : public _Vector_alloc<_Vec_base_types<_Ty, _Alloc>>\n"
                      "{     // varying size array of values\n"
                      "\n"
                      "  using reverse_iterator = _STD reverse_iterator<iterator>;\n"
                      "  using const_reverse_iterator = _STD reverse_iterator<const_iterator>;\n"
                      "  const_reverse_iterator crend() const noexcept\n"
                      "  {   // return iterator for end of reversed nonmutable sequence\n"
                      "    return (rend());\n"
                      "  }\n"
                      "};");

        ASSERT_EQUALS("", errout.str());
    }
};

REGISTER_TEST(TestSymbolDatabase)
