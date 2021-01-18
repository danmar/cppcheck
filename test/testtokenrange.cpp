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

#include "settings.h"
#include "testsuite.h"
#include "testutils.h"
#include "token.h"
#include "tokenize.h"
#include "tokenlist.h"
#include "tokenrange.h"
#include "symboldatabase.h"

#include <string>
#include <vector>

struct InternalError;


class TestTokenRange : public TestFixture {
public:
    TestTokenRange() : TestFixture("TestTokenRange") {
    }

private:
    void run() OVERRIDE {
        TEST_CASE(enumerationToEnd);
        TEST_CASE(toEndHelper);
        TEST_CASE(partialEnumeration);
        TEST_CASE(scopeExample);
        TEST_CASE(exampleAlgorithms);
    }

    void enumerationToEnd() const {
        givenACodeSampleToTokenize var("void a(){} void main(){ if(true){a();} }");
        const Token* expected = var.tokens();
        for (auto t : ConstTokenRange{var.tokens(), nullptr }) {
            ASSERT_EQUALS(expected, t);
            expected = t->next();
        }
        const Token* const terminator = nullptr;
        ASSERT_EQUALS(terminator, expected);
    }

    void toEndHelper() const {
        givenACodeSampleToTokenize var("void a(){} void main(){ if(true){a();} }");
        const Token* expected = var.tokens();
        for (auto t : var.tokens()->toEnd()) {
            ASSERT_EQUALS(expected, t);
            expected = t->next();
        }
        const Token* const terminator = nullptr;
        ASSERT_EQUALS(terminator, expected);
    }

    void partialEnumeration() const {
        givenACodeSampleToTokenize var("void a(){} void main(){ if(true){a();} }");
        const Token* expected = var.tokens()->tokAt(4);
        const Token* end = var.tokens()->tokAt(10);
        for (auto t : ConstTokenRange{ expected, end }) {
            ASSERT_EQUALS(expected, t);
            expected = t->next();
        }
        ASSERT_EQUALS(expected, end);
    }

    void scopeExample() const {
        Settings settings;
        Tokenizer tokenizer{ &settings, nullptr };
        std::istringstream sample("void a(){} void main(){ if(true){a();} }");
        tokenizer.tokenize(sample, "test.cpp");

        const SymbolDatabase* sd = tokenizer.getSymbolDatabase();
        const Scope& scope = *std::next(sd->scopeList.begin(), 3); //The scope of the if block

        std::ostringstream contents;
        for (auto t : ConstTokenRange{ scope.bodyStart->next(), scope.bodyEnd })
        {
            contents << t->str();
        }
        ASSERT_EQUALS("a();", contents.str());
    }

    void exampleAlgorithms() const {
        givenACodeSampleToTokenize var("void a(){} void main(){ if(true){a();} }");
        ConstTokenRange range{ var.tokens(), nullptr };
        ASSERT_EQUALS(true, std::all_of(range.begin(), range.end(), [](const Token*) {return true;}));
        ASSERT_EQUALS(true, std::any_of(range.begin(), range.end(), [](const Token* t) {return t->str() == "true";}));
        ASSERT_EQUALS("true", (*std::find_if(range.begin(), range.end(), [](const Token* t) {return t->str() == "true";}))->str());
        ASSERT_EQUALS(3, std::count_if(range.begin(), range.end(), [](const Token* t) {return t->str() == "{";}));
    }
};

REGISTER_TEST(TestTokenRange)
