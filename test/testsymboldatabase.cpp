/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2011 Daniel Marjamäki and Cppcheck team.
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

class TestSymbolDatabase: public TestFixture
{
public:
    TestSymbolDatabase()
        :TestFixture("TestSymbolDatabase")
        ,si(NULL, NULL, NULL)
        ,vartok(NULL)
        ,typetok(NULL)
        ,t(NULL)
        ,found(false)
        ,isArray(false)
    {}

private:
    const Scope si;
    const Token* vartok;
    const Token* typetok;
    const Token* t;
    bool found;
    bool isArray;

    void reset()
    {
        vartok = NULL;
        typetok = NULL;
        t = NULL;
        found = false;
        isArray = false;
    }

    void run()
    {
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

        TEST_CASE(hasGlobalVariables1);
        TEST_CASE(hasGlobalVariables2);
        TEST_CASE(hasGlobalVariables3);
    }

    void test_isVariableDeclarationCanHandleNull()
    {
        reset();
        bool result = si.isVariableDeclaration(NULL, vartok, typetok, isArray);
        ASSERT_EQUALS(false, result);
        ASSERT(NULL == vartok);
        ASSERT(NULL == typetok);
        ASSERT(false == isArray);
    }

    void test_isVariableDeclarationIdentifiesSimpleDeclaration()
    {
        reset();
        givenACodeSampleToTokenize simpleDeclaration("int x;");
        bool result = si.isVariableDeclaration(simpleDeclaration.tokens(), vartok, typetok, isArray);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("x", vartok->str());
        ASSERT_EQUALS("int", typetok->str());
        ASSERT(false == isArray);
    }

    void test_isVariableDeclarationIdentifiesScopedDeclaration()
    {
        reset();
        givenACodeSampleToTokenize ScopedDeclaration("::int x;");
        bool result = si.isVariableDeclaration(ScopedDeclaration.tokens(), vartok, typetok, isArray);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("x", vartok->str());
        ASSERT_EQUALS("int", typetok->str());
        ASSERT(false == isArray);
    }

    void test_isVariableDeclarationIdentifiesStdDeclaration()
    {
        reset();
        givenACodeSampleToTokenize StdDeclaration("std::string x;");
        bool result = si.isVariableDeclaration(StdDeclaration.tokens(), vartok, typetok, isArray);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("x", vartok->str());
        ASSERT_EQUALS("string", typetok->str());
        ASSERT(false == isArray);
    }

    void test_isVariableDeclarationIdentifiesScopedStdDeclaration()
    {
        reset();
        givenACodeSampleToTokenize StdDeclaration("::std::string x;");
        bool result = si.isVariableDeclaration(StdDeclaration.tokens(), vartok, typetok, isArray);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("x", vartok->str());
        ASSERT_EQUALS("string", typetok->str());
        ASSERT(false == isArray);
    }

    void test_isVariableDeclarationIdentifiesManyScopes()
    {
        reset();
        givenACodeSampleToTokenize manyScopes("AA::BB::CC::DD::EE x;");
        bool result = si.isVariableDeclaration(manyScopes.tokens(), vartok, typetok, isArray);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("x", vartok->str());
        ASSERT_EQUALS("EE", typetok->str());
        ASSERT(false == isArray);
    }

    void test_isVariableDeclarationIdentifiesPointers()
    {
        reset();
        givenACodeSampleToTokenize pointer("int* p;");
        bool result = si.isVariableDeclaration(pointer.tokens(), vartok, typetok, isArray);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("p", vartok->str());
        ASSERT_EQUALS("int", typetok->str());
        ASSERT(false == isArray);
    }

    void test_isVariableDeclarationDoesNotIdentifyConstness()
    {
        reset();
        givenACodeSampleToTokenize constness("const int* cp;");
        bool result = si.isVariableDeclaration(constness.tokens(), vartok, typetok, isArray);
        ASSERT_EQUALS(false, result);
        ASSERT(NULL == vartok);
        ASSERT(NULL == typetok);
        ASSERT(false == isArray);
    }

    void test_isVariableDeclarationIdentifiesFirstOfManyVariables()
    {
        reset();
        givenACodeSampleToTokenize multipleDeclaration("int first, second;");
        bool result = si.isVariableDeclaration(multipleDeclaration.tokens(), vartok, typetok, isArray);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("first", vartok->str());
        ASSERT_EQUALS("int", typetok->str());
        ASSERT(false == isArray);
    }

    void test_isVariableDeclarationIdentifiesScopedPointerDeclaration()
    {
        reset();
        givenACodeSampleToTokenize manyScopes("AA::BB::CC::DD::EE* p;");
        bool result = si.isVariableDeclaration(manyScopes.tokens(), vartok, typetok, isArray);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("p", vartok->str());
        ASSERT_EQUALS("EE", typetok->str());
        ASSERT(false == isArray);
    }

    void test_isVariableDeclarationIdentifiesDeclarationWithIndirection()
    {
        reset();
        givenACodeSampleToTokenize pointerToPointer("int** pp;");
        bool result = si.isVariableDeclaration(pointerToPointer.tokens(), vartok, typetok, isArray);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("pp", vartok->str());
        ASSERT_EQUALS("int", typetok->str());
        ASSERT(false == isArray);
    }

    void test_isVariableDeclarationIdentifiesDeclarationWithMultipleIndirection()
    {
        reset();
        givenACodeSampleToTokenize pointerToPointer("int***** p;");
        bool result = si.isVariableDeclaration(pointerToPointer.tokens(), vartok, typetok, isArray);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("p", vartok->str());
        ASSERT_EQUALS("int", typetok->str());
        ASSERT(false == isArray);
    }

    void test_isVariableDeclarationIdentifiesArray()
    {
        reset();
        givenACodeSampleToTokenize array("::std::string v[3];");
        bool result = si.isVariableDeclaration(array.tokens(), vartok, typetok, isArray);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("v", vartok->str());
        ASSERT_EQUALS("string", typetok->str());
        ASSERT(true == isArray);
    }

    void test_isVariableDeclarationIdentifiesOfArrayPointers()
    {
        reset();
        givenACodeSampleToTokenize array("A *a[5];");
        bool result = si.isVariableDeclaration(array.tokens(), vartok, typetok, isArray);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("a", vartok->str());
        ASSERT_EQUALS("A", typetok->str());
        ASSERT(true == isArray);
    }

    void isVariableDeclarationIdentifiesTemplatedPointerVariable()
    {
        reset();
        givenACodeSampleToTokenize var("std::set<char>* chars;");
        bool result = si.isVariableDeclaration(var.tokens(), vartok, typetok, isArray);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("chars", vartok->str());
        ASSERT_EQUALS("set", typetok->str());
        ASSERT(false == isArray);
    }

    void isVariableDeclarationIdentifiesTemplatedPointerToPointerVariable()
    {
        reset();
        givenACodeSampleToTokenize var("std::deque<int>*** ints;");
        bool result = si.isVariableDeclaration(var.tokens(), vartok, typetok, isArray);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("ints", vartok->str());
        ASSERT_EQUALS("deque", typetok->str());
        ASSERT(false == isArray);
    }

    void isVariableDeclarationIdentifiesTemplatedArrayVariable()
    {
        reset();
        givenACodeSampleToTokenize var("std::deque<int> ints[3];");
        bool result = si.isVariableDeclaration(var.tokens(), vartok, typetok, isArray);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("ints", vartok->str());
        ASSERT_EQUALS("deque", typetok->str());
        ASSERT(true == isArray);
    }

    void isVariableDeclarationIdentifiesTemplatedVariable()
    {
        reset();
        givenACodeSampleToTokenize var("std::vector<int> ints;");
        bool result = si.isVariableDeclaration(var.tokens(), vartok, typetok, isArray);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("ints", vartok->str());
        ASSERT_EQUALS("vector", typetok->str());
        ASSERT(false == isArray);
    }

    void isVariableDeclarationIdentifiesTemplatedVariableIterator()
    {
        reset();
        givenACodeSampleToTokenize var("std::list<int>::const_iterator floats;");
        bool result = si.isVariableDeclaration(var.tokens(), vartok, typetok, isArray);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("floats", vartok->str());
        ASSERT_EQUALS("const_iterator", typetok->str());
        ASSERT(false == isArray);
    }

    void isVariableDeclarationIdentifiesNestedTemplateVariable()
    {
        reset();
        givenACodeSampleToTokenize var("std::deque<std::set<int> > intsets;");
        bool result = si.isVariableDeclaration(var.tokens(), vartok, typetok, isArray);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("intsets", vartok->str());
        ASSERT_EQUALS("deque", typetok->str());
        ASSERT(false == isArray);
    }

    void isVariableDeclarationDoesNotIdentifyTemplateClass()
    {
        reset();
        givenACodeSampleToTokenize var("template <class T> class SomeClass{};");
        bool result = si.isVariableDeclaration(var.tokens(), vartok, typetok, isArray);
        ASSERT_EQUALS(false, result);
        ASSERT(false == isArray);
    }

    void canFindMatchingBracketsNeedsOpen()
    {
        reset();
        givenACodeSampleToTokenize var("std::deque<std::set<int> > intsets;");

        found = si.findClosingBracket(var.tokens(), t);
        ASSERT(! found);
        ASSERT(! t);
    }

    void canFindMatchingBracketsInnerPair()
    {
        reset();
        givenACodeSampleToTokenize var("std::deque<std::set<int> > intsets;");

        found = si.findClosingBracket(var.tokens()->tokAt(7), t);
        ASSERT(found);
        ASSERT_EQUALS(">", t->str());
        ASSERT_EQUALS(var.tokens()->strAt(9), t->str());
    }

    void canFindMatchingBracketsOuterPair()
    {
        reset();
        givenACodeSampleToTokenize var("std::deque<std::set<int> > intsets;");

        found = si.findClosingBracket(var.tokens()->tokAt(3), t);
        ASSERT(found);
        ASSERT_EQUALS(">", t->str());
        ASSERT_EQUALS(var.tokens()->strAt(10), t->str());

    }

    void canFindMatchingBracketsWithTooManyClosing()
    {
        reset();
        givenACodeSampleToTokenize var("X< 1>2 > x1;\n");

        found = si.findClosingBracket(var.tokens()->tokAt(1), t);
        ASSERT(found);
        ASSERT_EQUALS(">", t->str());
        ASSERT_EQUALS(var.tokens()->strAt(3), t->str());
    }

    void canFindMatchingBracketsWithTooManyOpening()
    {
        reset();
        givenACodeSampleToTokenize var("X < (2 < 1) > x1;\n");

        found = si.findClosingBracket(var.tokens()->tokAt(1), t);
        ASSERT(!found);
    }

    void hasRegularFunction()
    {
        GET_SYMBOL_DB("void func() { }\n")

        // 2 scopes: Global and Function
        ASSERT(db && db->scopeList.size() == 2 && tokenizer.getFunctionTokenByName("func"));

        if (db)
        {
            const Scope *scope = db->findFunctionScopeByToken(tokenizer.tokens()->tokAt(1));

            ASSERT(scope && scope->className == "func");

            const Function *function = db->findFunctionByToken(tokenizer.tokens()->tokAt(1));

            ASSERT(function && function->token->str() == "func");
            ASSERT(function && function->token == tokenizer.tokens()->tokAt(1));
            ASSERT(function && function->hasBody);
        }
    }

    void hasInlineClassFunction()
    {
        GET_SYMBOL_DB("class Fred { void func() { } };\n")

        // 3 scopes: Global, Class, and Function
        ASSERT(db && db->scopeList.size() == 3 && tokenizer.getFunctionTokenByName("func"));

        if (db)
        {
            const Scope *scope = db->findFunctionScopeByToken(tokenizer.tokens()->tokAt(4));

            ASSERT(scope && scope->className == "func");

            const Function *function = db->findFunctionByToken(tokenizer.tokens()->tokAt(4));

            ASSERT(function && function->token->str() == "func");
            ASSERT(function && function->token == tokenizer.tokens()->tokAt(4));
            ASSERT(function && function->hasBody && function->isInline);
        }
    }

    void hasMissingInlineClassFunction()
    {
        GET_SYMBOL_DB("class Fred { void func(); };\n")

        // 2 scopes: Global and Class (no Function scope because there is no function implementation)
        ASSERT(db && db->scopeList.size() == 2 && !tokenizer.getFunctionTokenByName("func"));

        if (db)
        {
            const Scope *scope = db->findFunctionScopeByToken(tokenizer.tokens()->tokAt(4));

            ASSERT(scope == NULL);

            const Function *function = db->findFunctionByToken(tokenizer.tokens()->tokAt(4));

            ASSERT(function && function->token->str() == "func");
            ASSERT(function && function->token == tokenizer.tokens()->tokAt(4));
            ASSERT(function && !function->hasBody);
        }
    }

    void hasClassFunction()
    {
        GET_SYMBOL_DB("class Fred { void func(); }; Fred::func() { }\n")

        // 3 scopes: Global, Class, and Function
        ASSERT(db && db->scopeList.size() == 3 && tokenizer.getFunctionTokenByName("func"));

        if (db)
        {
            const Scope *scope = db->findFunctionScopeByToken(tokenizer.tokens()->tokAt(12));

            ASSERT(scope && scope->className == "func");

            const Function *function = db->findFunctionByToken(tokenizer.tokens()->tokAt(12));

            ASSERT(function && function->token->str() == "func");
            ASSERT(function && function->token == tokenizer.tokens()->tokAt(12));
            ASSERT(function && function->hasBody && !function->isInline);
        }
    }

    void hasRegularFunctionReturningFunctionPointer()
    {
        GET_SYMBOL_DB("void (*func(int f))(char) { }\n")

        // 2 scopes: Global and Function
        ASSERT(db && db->scopeList.size() == 2 && tokenizer.getFunctionTokenByName("func"));

        if (db)
        {
            const Scope *scope = db->findFunctionScopeByToken(tokenizer.tokens()->tokAt(3));

            ASSERT(scope && scope->className == "func");

            const Function *function = db->findFunctionByToken(tokenizer.tokens()->tokAt(3));

            ASSERT(function && function->token->str() == "func");
            ASSERT(function && function->token == tokenizer.tokens()->tokAt(3));
            ASSERT(function && function->hasBody && function->retFuncPtr);
        }
    }

    void hasInlineClassFunctionReturningFunctionPointer()
    {
        GET_SYMBOL_DB("class Fred { void (*func(int f))(char) { } };\n")

        // 3 scopes: Global, Class, and Function
        ASSERT(db && db->scopeList.size() == 3 && tokenizer.getFunctionTokenByName("func"));

        if (db)
        {
            const Scope *scope = db->findFunctionScopeByToken(tokenizer.tokens()->tokAt(6));

            ASSERT(scope && scope->className == "func");

            const Function *function = db->findFunctionByToken(tokenizer.tokens()->tokAt(6));

            ASSERT(function && function->token->str() == "func");
            ASSERT(function && function->token == tokenizer.tokens()->tokAt(6));
            ASSERT(function && function->hasBody && function->isInline && function->retFuncPtr);
        }
    }

    void hasMissingInlineClassFunctionReturningFunctionPointer()
    {
        GET_SYMBOL_DB("class Fred { void (*func(int f))(char); };\n")

        // 2 scopes: Global and Class (no Function scope because there is no function implementation)
        ASSERT(db && db->scopeList.size() == 2 && !tokenizer.getFunctionTokenByName("func"));

        if (db)
        {
            const Scope *scope = db->findFunctionScopeByToken(tokenizer.tokens()->tokAt(6));

            ASSERT(scope == NULL);

            const Function *function = db->findFunctionByToken(tokenizer.tokens()->tokAt(6));

            ASSERT(function && function->token->str() == "func");
            ASSERT(function && function->token == tokenizer.tokens()->tokAt(6));
            ASSERT(function && !function->hasBody && function->retFuncPtr);
        }
    }

    void hasClassFunctionReturningFunctionPointer()
    {
        GET_SYMBOL_DB("class Fred { void (*func(int f))(char); }; void (*Fred::func(int f))(char) { }\n")

        // 3 scopes: Global, Class, and Function
        ASSERT(db && db->scopeList.size() == 3 && tokenizer.getFunctionTokenByName("func"));

        if (db)
        {
            const Scope *scope = db->findFunctionScopeByToken(tokenizer.tokens()->tokAt(23));

            ASSERT(scope && scope->className == "func");

            const Function *function = db->findFunctionByToken(tokenizer.tokens()->tokAt(23));

            ASSERT(function && function->token->str() == "func");
            ASSERT(function && function->token == tokenizer.tokens()->tokAt(23));
            ASSERT(function && function->hasBody && !function->isInline && function->retFuncPtr);
        }
    }

    void hasGlobalVariables1()
    {
        GET_SYMBOL_DB("int i;\n")

        ASSERT(db && db->scopeList.size() == 1);
        if (db && db->scopeList.size() == 1)
        {
            std::list<Scope *>::const_iterator it = db->scopeList.begin();
            ASSERT((*it)->varlist.size() == 1);
            if ((*it)->varlist.size() == 1)
            {
                std::list<Variable>::const_iterator var = (*it)->varlist.begin();
                ASSERT(var->name() == "i");
                ASSERT(var->typeStartToken()->str() == "int");
            }
        }
    }

    void hasGlobalVariables2()
    {
        GET_SYMBOL_DB("int array[2][2];\n")

        ASSERT(db && db->scopeList.size() == 1);
        if (db && db->scopeList.size() == 1)
        {
            std::list<Scope *>::const_iterator it = db->scopeList.begin();
            ASSERT((*it)->varlist.size() == 1);
            if ((*it)->varlist.size() == 1)
            {
                std::list<Variable>::const_iterator var = (*it)->varlist.begin();
                ASSERT(var->name() == "array");
                ASSERT(var->typeStartToken()->str() == "int");
            }
        }
    }

    void hasGlobalVariables3()
    {
        GET_SYMBOL_DB("int array[2][2] = { { 0, 0 }, { 0, 0 } };\n")

        ASSERT(db && db->scopeList.size() == 1);
        if (db && db->scopeList.size() == 1)
        {
            std::list<Scope *>::const_iterator it = db->scopeList.begin();
            ASSERT((*it)->varlist.size() == 1);
            if ((*it)->varlist.size() == 1)
            {
                std::list<Variable>::const_iterator var = (*it)->varlist.begin();
                ASSERT(var->name() == "array");
                ASSERT(var->typeStartToken()->str() == "int");
            }
        }
    }
};

REGISTER_TEST(TestSymbolDatabase)

