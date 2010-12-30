/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2010 Daniel Marjamäki and Cppcheck team.
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
    {}

private:
    const SymbolDatabase::SpaceInfo si;
    const Token* vartok;

    void run()
    {
        TEST_CASE(test_isVariableDeclarationCanHandleNull);
        TEST_CASE(test_isVariableDeclarationIdentifiesSimpleDeclaration);
        TEST_CASE(test_isVariableDeclarationIdentifiesScopedDeclaration);
        TEST_CASE(test_isVariableDeclarationIdentifiesStdDeclaration);
        TEST_CASE(test_isVariableDeclarationIdentifiesScopedStdDeclaration);
        TEST_CASE(test_isVariableDeclarationIdentifiesManyScopes);
        TEST_CASE(test_isVariableDeclarationDoesNotIdentifyPointers);
        TEST_CASE(test_isVariableDeclarationDoesNotIdentifyConstness);
        TEST_CASE(test_isVariableDeclarationIdentifiesFirstOfManyVariables);
    }

    void test_isVariableDeclarationCanHandleNull()
    {
        vartok = NULL;
        bool result = si.isVariableDeclaration(NULL, vartok);
        ASSERT_EQUALS(false, result);
        ASSERT(NULL == vartok);
    }

    void test_isVariableDeclarationIdentifiesSimpleDeclaration()
    {
        vartok = NULL;
        givenACodeSampleToTokenize simpleDeclaration("int x;");
        bool result = si.isVariableDeclaration(simpleDeclaration.tokens(), vartok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("x", vartok->str());
    }

    void test_isVariableDeclarationIdentifiesScopedDeclaration()
    {
        vartok = NULL;
        givenACodeSampleToTokenize ScopedDeclaration("::int x;");
        bool result = si.isVariableDeclaration(ScopedDeclaration.tokens(), vartok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("x", vartok->str());
    }

    void test_isVariableDeclarationIdentifiesStdDeclaration()
    {
        vartok = NULL;
        givenACodeSampleToTokenize StdDeclaration("std::string x;");
        bool result = si.isVariableDeclaration(StdDeclaration.tokens(), vartok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("x", vartok->str());
    }

    void test_isVariableDeclarationIdentifiesScopedStdDeclaration()
    {
        vartok = NULL;
        givenACodeSampleToTokenize StdDeclaration("::std::string x;");
        bool result = si.isVariableDeclaration(StdDeclaration.tokens(), vartok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("x", vartok->str());
    }

    void test_isVariableDeclarationIdentifiesManyScopes()
    {
        vartok = NULL;
        givenACodeSampleToTokenize manyScopes("AA::BB::CC::DD::EE x;");
        bool result = si.isVariableDeclaration(manyScopes.tokens(), vartok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("x", vartok->str());
    }

    void test_isVariableDeclarationDoesNotIdentifyPointers()
    {
        vartok = NULL;
        givenACodeSampleToTokenize pointer("int* p;");
        bool result = si.isVariableDeclaration(pointer.tokens(), vartok);
        ASSERT_EQUALS(false, result);
        ASSERT(NULL == vartok);
    }

    void test_isVariableDeclarationDoesNotIdentifyConstness()
    {
        vartok = NULL;
        givenACodeSampleToTokenize constness("const int* cp;");
        bool result = si.isVariableDeclaration(constness.tokens(), vartok);
        ASSERT_EQUALS(false, result);
        ASSERT(NULL == vartok);
    }

    void test_isVariableDeclarationIdentifiesFirstOfManyVariables()
    {
        vartok = NULL;
        givenACodeSampleToTokenize multipleDeclaration("int first, second;");
        bool result = si.isVariableDeclaration(multipleDeclaration.tokens(), vartok);
        ASSERT_EQUALS(true, result);
        ASSERT_EQUALS("first", vartok->str());
    }
};

REGISTER_TEST(TestSymbolDatabase)

