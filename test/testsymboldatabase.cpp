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
#define private public
#include "symboldatabase.h"

class TestSymbolDatabase: public TestFixture
{
public:
    TestSymbolDatabase()
        :TestFixture("TestSymbolDatabase")
        ,si(NULL, NULL, NULL)
        ,vartok(NULL)
        ,typetok(NULL)
    {}

private:
    const SymbolDatabase::SpaceInfo si;
    const Token* vartok;
    const Token* typetok;
    const Token* t;
    bool found;

    void reset()
    {
        vartok = NULL;
        typetok = NULL;
        t = NULL;
        found = false;
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
        TEST_CASE(isVariableDeclarationIdentifiesTemplatedVariable);
        TEST_CASE(isVariableDeclarationIdentifiesTemplatedVariableIterator);
        TEST_CASE(isVariableDeclarationIdentifiesNestedTemplateVariable);
        TEST_CASE(isVariableDeclarationDoesNotIdentifyTemplateClass);
        TEST_CASE(canFindMatchingBracketsNeedsOpen);
        TEST_CASE(canFindMatchingBracketsInnerPair);
        TEST_CASE(canFindMatchingBracketsOuterPair);
        TEST_CASE(canFindMatchingBracketsWithTooManyClosing);
        TEST_CASE(canFindMatchingBracketsWithTooManyOpening);
    }

    void test_isVariableDeclarationCanHandleNull()
    {
        reset();
        bool result = si.isVariableDeclaration(NULL, vartok, typetok);
        ASSERT_EQUALS(false, result);
        ASSERT(NULL == vartok);
        ASSERT(NULL == typetok);
    }

    void test_isVariableDeclarationIdentifiesSimpleDeclaration()
    {
        reset();
        givenACodeSampleToTokenize simpleDeclaration("int x;");
        bool result = si.isVariableDeclaration(simpleDeclaration.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("x", vartok->str());
        ASSERT_EQUALS("int", typetok->str());
    }

    void test_isVariableDeclarationIdentifiesScopedDeclaration()
    {
        reset();
        givenACodeSampleToTokenize ScopedDeclaration("::int x;");
        bool result = si.isVariableDeclaration(ScopedDeclaration.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("x", vartok->str());
        ASSERT_EQUALS("int", typetok->str());
    }

    void test_isVariableDeclarationIdentifiesStdDeclaration()
    {
        reset();
        givenACodeSampleToTokenize StdDeclaration("std::string x;");
        bool result = si.isVariableDeclaration(StdDeclaration.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("x", vartok->str());
        ASSERT_EQUALS("string", typetok->str());
    }

    void test_isVariableDeclarationIdentifiesScopedStdDeclaration()
    {
        reset();
        givenACodeSampleToTokenize StdDeclaration("::std::string x;");
        bool result = si.isVariableDeclaration(StdDeclaration.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("x", vartok->str());
        ASSERT_EQUALS("string", typetok->str());
    }

    void test_isVariableDeclarationIdentifiesManyScopes()
    {
        reset();
        givenACodeSampleToTokenize manyScopes("AA::BB::CC::DD::EE x;");
        bool result = si.isVariableDeclaration(manyScopes.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("x", vartok->str());
        ASSERT_EQUALS("EE", typetok->str());
    }

    void test_isVariableDeclarationIdentifiesPointers()
    {
        reset();
        givenACodeSampleToTokenize pointer("int* p;");
        bool result = si.isVariableDeclaration(pointer.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("p", vartok->str());
        ASSERT_EQUALS("int", typetok->str());
    }

    void test_isVariableDeclarationDoesNotIdentifyConstness()
    {
        reset();
        givenACodeSampleToTokenize constness("const int* cp;");
        bool result = si.isVariableDeclaration(constness.tokens(), vartok, typetok);
        ASSERT_EQUALS(false, result);
        ASSERT(NULL == vartok);
        ASSERT(NULL == typetok);
    }

    void test_isVariableDeclarationIdentifiesFirstOfManyVariables()
    {
        reset();
        givenACodeSampleToTokenize multipleDeclaration("int first, second;");
        bool result = si.isVariableDeclaration(multipleDeclaration.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("first", vartok->str());
        ASSERT_EQUALS("int", typetok->str());
    }

    void test_isVariableDeclarationIdentifiesScopedPointerDeclaration()
    {
        reset();
        givenACodeSampleToTokenize manyScopes("AA::BB::CC::DD::EE* p;");
        bool result = si.isVariableDeclaration(manyScopes.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("p", vartok->str());
        ASSERT_EQUALS("EE", typetok->str());
    }

    void test_isVariableDeclarationIdentifiesDeclarationWithIndirection()
    {
        reset();
        givenACodeSampleToTokenize pointerToPointer("int** pp;");
        bool result = si.isVariableDeclaration(pointerToPointer.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("pp", vartok->str());
        ASSERT_EQUALS("int", typetok->str());
    }

    void test_isVariableDeclarationIdentifiesDeclarationWithMultipleIndirection()
    {
        reset();
        givenACodeSampleToTokenize pointerToPointer("int***** p;");
        bool result = si.isVariableDeclaration(pointerToPointer.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("p", vartok->str());
        ASSERT_EQUALS("int", typetok->str());
    }

    void test_isVariableDeclarationIdentifiesArray()
    {
        reset();
        givenACodeSampleToTokenize array("::std::string v[3];");
        bool result = si.isVariableDeclaration(array.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("v", vartok->str());
        ASSERT_EQUALS("string", typetok->str());
    }

    void test_isVariableDeclarationIdentifiesOfArrayPointers()
    {
        reset();
        givenACodeSampleToTokenize array("A *a[5];");
        bool result = si.isVariableDeclaration(array.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("a", vartok->str());
        ASSERT_EQUALS("A", typetok->str());
    }

    void isVariableDeclarationIdentifiesTemplatedPointerVariable()
    {
        reset();
        givenACodeSampleToTokenize var("std::set<char>* chars;");
        bool result = si.isVariableDeclaration(var.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("chars", vartok->str());
        ASSERT_EQUALS("set", typetok->str());
    }

    void isVariableDeclarationIdentifiesTemplatedPointerToPointerVariable()
    {
        reset();
        givenACodeSampleToTokenize var("std::deque<int>*** ints;");
        bool result = si.isVariableDeclaration(var.tokens(), vartok, typetok);
        TODO_ASSERT_EQUALS(true, result);
        if (NULL != vartok) 
        {
            TODO_ASSERT_EQUALS("ints", vartok->str());
        }
        if (NULL != typetok)
        {
            TODO_ASSERT_EQUALS("deque", typetok->str());
        }
    }

    void isVariableDeclarationIdentifiesTemplatedVariable()
    {
        reset();
        givenACodeSampleToTokenize var("std::vector<int> ints;");
        bool result = si.isVariableDeclaration(var.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("ints", vartok->str());
        ASSERT_EQUALS("vector", typetok->str());
    }

    void isVariableDeclarationIdentifiesTemplatedVariableIterator()
    {
        reset();
        givenACodeSampleToTokenize var("std::list<int>::const_iterator floats;");
        bool result = si.isVariableDeclaration(var.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("floats", vartok->str());
        ASSERT_EQUALS("const_iterator", typetok->str());
    }

    void isVariableDeclarationIdentifiesNestedTemplateVariable()
    {
        reset();
        givenACodeSampleToTokenize var("std::deque<std::set<int> > intsets;");
        bool result = si.isVariableDeclaration(var.tokens(), vartok, typetok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("intsets", vartok->str());
        ASSERT_EQUALS("deque", typetok->str());
    }

    void isVariableDeclarationDoesNotIdentifyTemplateClass()
    {
        reset();
        givenACodeSampleToTokenize var("template <class T> class SomeClass{};");
        bool result = si.isVariableDeclaration(var.tokens(), vartok, typetok);
        ASSERT_EQUALS(false, result);
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
};

REGISTER_TEST(TestSymbolDatabase)

