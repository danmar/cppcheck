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

#include "settings.h"
#include "fixture.h"
#include "platform.h"
#include "token.h"
#include "tokenlist.h"

#include <sstream> // IWYU pragma: keep
#include <string>

class TestTokenList : public TestFixture {
public:
    TestTokenList() : TestFixture("TestTokenList") {}

private:
    const Settings settings;

    void run() override {
        TEST_CASE(testaddtoken1);
        TEST_CASE(testaddtoken2);
        TEST_CASE(inc);
        TEST_CASE(isKeyword);
    }

    // inspired by #5895
    void testaddtoken1() const {
        const std::string code = "0x89504e470d0a1a0a";
        TokenList tokenlist(&settings);
        tokenlist.addtoken(code, 1, 1, false);
        ASSERT_EQUALS("0x89504e470d0a1a0a", tokenlist.front()->str());
    }

    void testaddtoken2() const {
        const std::string code = "0xF0000000";
        Settings settings1;
        settings1.platform.int_bit = 32;
        TokenList tokenlist(&settings1);
        tokenlist.addtoken(code, 1, 1, false);
        ASSERT_EQUALS("0xF0000000", tokenlist.front()->str());
    }

    void inc() const {
        const char code[] = "a++1;1++b;";

        errout.str("");

        // tokenize..
        TokenList tokenlist(&settings);
        std::istringstream istr(code);
        tokenlist.createTokens(istr, "a.cpp");

        ASSERT(Token::simpleMatch(tokenlist.front(), "a + + 1 ; 1 + + b ;"));
    }

    void isKeyword() const {

        const char code[] = "for a int delete true";

        {
            TokenList tokenlist(&settings);
            std::istringstream istr(code);
            tokenlist.createTokens(istr, "a.c");

            ASSERT_EQUALS(true, tokenlist.front()->isKeyword());
            ASSERT_EQUALS(true, tokenlist.front()->isControlFlowKeyword());
            ASSERT_EQUALS(false, tokenlist.front()->next()->isKeyword());
            ASSERT_EQUALS(false, tokenlist.front()->next()->isControlFlowKeyword());
            ASSERT_EQUALS(false, tokenlist.front()->tokAt(2)->isKeyword());
            ASSERT_EQUALS(true, tokenlist.front()->tokAt(2)->tokType() == Token::eType);
            ASSERT_EQUALS(false, tokenlist.front()->tokAt(2)->isControlFlowKeyword());
            ASSERT_EQUALS(false, tokenlist.front()->tokAt(3)->isKeyword());
            ASSERT_EQUALS(false, tokenlist.front()->tokAt(3)->isControlFlowKeyword());
            ASSERT_EQUALS(false, tokenlist.front()->tokAt(4)->isKeyword());
            ASSERT_EQUALS(true, tokenlist.front()->tokAt(4)->isLiteral());
            ASSERT_EQUALS(false, tokenlist.front()->tokAt(4)->isControlFlowKeyword());
        }
        {
            TokenList tokenlist(&settings);
            std::istringstream istr(code);
            tokenlist.createTokens(istr, "a.cpp");

            ASSERT_EQUALS(true, tokenlist.front()->isKeyword());
            ASSERT_EQUALS(true, tokenlist.front()->isControlFlowKeyword());
            ASSERT_EQUALS(false, tokenlist.front()->next()->isKeyword());
            ASSERT_EQUALS(false, tokenlist.front()->next()->isControlFlowKeyword());
            ASSERT_EQUALS(false, tokenlist.front()->tokAt(2)->isKeyword());
            ASSERT_EQUALS(true, tokenlist.front()->tokAt(2)->tokType() == Token::eType);
            ASSERT_EQUALS(false, tokenlist.front()->tokAt(2)->isControlFlowKeyword());
            ASSERT_EQUALS(true, tokenlist.front()->tokAt(3)->isKeyword());
            ASSERT_EQUALS(false, tokenlist.front()->tokAt(3)->isControlFlowKeyword());
            ASSERT_EQUALS(false, tokenlist.front()->tokAt(4)->isKeyword());
            ASSERT_EQUALS(true, tokenlist.front()->tokAt(4)->isLiteral());
            ASSERT_EQUALS(false, tokenlist.front()->tokAt(4)->isControlFlowKeyword());
        }
    }
};

REGISTER_TEST(TestTokenList)
