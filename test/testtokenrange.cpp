/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2021 Cppcheck team.
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
    TestTokenRange() : TestFixture("TestTokenRange") {}

private:
    void run() OVERRIDE {
        TEST_CASE(enumerationToEnd);
        TEST_CASE(untilHelperToEnd);
        TEST_CASE(untilHelperPartWay);
        TEST_CASE(partialEnumeration);
        TEST_CASE(scopeExample);
        TEST_CASE(exampleAlgorithms);
    }

    std::string testTokenRange(ConstTokenRange range, const Token* start, const Token* end) const {
        auto tokenToString = [](const Token* t) {
            return t ? t->str() : "<null>";
        };
        int index = 0;
        const Token* expected = start;
        for (const Token* t : range) {
            if (expected != t) {
                std::ostringstream message;
                message << "Failed to match token " << tokenToString(expected) << " at position " << index << ". Got " << tokenToString(t) << " instead";
                return message.str();
            }
            index++;
            expected = expected->next();
        }
        if (expected != end) {
            std::ostringstream message;
            message << "Failed to terminate on " << tokenToString(end) << ". Instead terminated on " << tokenToString(expected) << " at position " << index << ".";
            return message.str();
        }
        return "";
    }

    void enumerationToEnd() const {
        std::istringstream istr("void a(){} void main(){ if(true){a();} }");
        TokenList tokenList(nullptr);
        tokenList.createTokens(istr, "test.cpp");
        ASSERT_EQUALS("", testTokenRange(ConstTokenRange{ tokenList.front(), nullptr }, tokenList.front(), nullptr));
    }

    void untilHelperToEnd() const {
        std::istringstream istr("void a(){} void main(){ if(true){a();} }");
        TokenList tokenList(nullptr);
        tokenList.createTokens(istr, "test.cpp");
        ASSERT_EQUALS("", testTokenRange(tokenList.front()->until(nullptr), tokenList.front(), nullptr));
    }

    void untilHelperPartWay() const {
        std::istringstream istr("void a(){} void main(){ if(true){a();} }");
        TokenList tokenList(nullptr);
        tokenList.createTokens(istr, "test.cpp");
        const Token* start = tokenList.front()->tokAt(4);
        const Token* end = start->tokAt(8);
        ASSERT_EQUALS("", testTokenRange(start->until(end), start, end));
    }

    void partialEnumeration() const {
        std::istringstream istr("void a(){} void main(){ if(true){a();} }");
        TokenList tokenList(nullptr);
        tokenList.createTokens(istr, "test.cpp");
        const Token* start = tokenList.front()->tokAt(4);
        const Token* end = tokenList.front()->tokAt(10);
        ASSERT_EQUALS("", testTokenRange(ConstTokenRange{ start, end }, start, end));
    }

    void scopeExample() const {
        Settings settings;
        Tokenizer tokenizer{ &settings, nullptr };
        std::istringstream sample("void a(){} void main(){ if(true){a();} }");
        tokenizer.tokenize(sample, "test.cpp");

        const SymbolDatabase* sd = tokenizer.getSymbolDatabase();
        const Scope& scope = *std::next(sd->scopeList.begin(), 3); //The scope of the if block

        std::ostringstream contents;
        for (const Token* t : ConstTokenRange{ scope.bodyStart->next(), scope.bodyEnd }) {
            contents << t->str();
        }
        ASSERT_EQUALS("a();", contents.str());
    }

    void exampleAlgorithms() const {
        std::istringstream istr("void a(){} void main(){ if(true){a();} }");
        TokenList tokenList(nullptr);
        tokenList.createTokens(istr, "test.cpp");
        ConstTokenRange range{ tokenList.front(), nullptr };
        ASSERT_EQUALS(true, std::all_of(range.begin(), range.end(), [](const Token*) {
            return true;
        }));
        ASSERT_EQUALS(true, std::any_of(range.begin(), range.end(), [](const Token* t) {
            return t->str() == "true";
        }));
        ASSERT_EQUALS("true", (*std::find_if(range.begin(), range.end(), [](const Token* t) {
            return t->str() == "true";
        }))->str());
        ASSERT_EQUALS(3, std::count_if(range.begin(), range.end(), [](const Token* t) {
            return t->str() == "{";
        }));
    }
};

REGISTER_TEST(TestTokenRange)
