/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki and Cppcheck team.
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/
 */

#include <cstring>
#include "testsuite.h"
#include "../src/tokenize.h"
#include "../src/token.h"

extern std::ostringstream errout;
class TestTOKEN : public TestFixture
{
public:
    TestTOKEN() : TestFixture("TestTOKEN")
    { }

private:

    void run()
    {
        TEST_CASE(nextprevious);
        TEST_CASE(multiCompare);
        TEST_CASE(getStrLength);
    }

    void nextprevious()
    {
        Token *token = new Token;
        token->str("1");
        token->insertToken("2");
        token->next()->insertToken("3");
        Token *last = token->tokAt(2);
        ASSERT_EQUALS(token->str(), "1");
        ASSERT_EQUALS(token->next()->str(), "2");
        ASSERT_EQUALS(token->tokAt(2)->str(), "3");
        if (last->next())
            ASSERT_EQUALS("Null was expected", "");

        ASSERT_EQUALS(last->str(), "3");
        ASSERT_EQUALS(last->previous()->str(), "2");
        ASSERT_EQUALS(last->tokAt(-2)->str(), "1");
        if (token->previous())
            ASSERT_EQUALS("Null was expected", "");

        Tokenizer::deleteTokens(token);
    }

    void multiCompare()
    {
        // Test for found
        ASSERT_EQUALS(1, Token::multiCompare("one|two", "one"));
        ASSERT_EQUALS(1, Token::multiCompare("one|two", "two"));
        ASSERT_EQUALS(1, Token::multiCompare("verybig|two|", "two"));

        // Test for empty string found
        ASSERT_EQUALS(0, Token::multiCompare("|one|two", "notfound"));
        ASSERT_EQUALS(0, Token::multiCompare("one||two", "notfound"));
        ASSERT_EQUALS(0, Token::multiCompare("one|two|", "notfound"));

        // Test for not found
        ASSERT_EQUALS(static_cast<unsigned int>(-1), static_cast<unsigned int>(Token::multiCompare("one|two", "notfound")));
        ASSERT_EQUALS(static_cast<unsigned int>(-1), static_cast<unsigned int>(Token::multiCompare("verybig|two", "s")));
        ASSERT_EQUALS(static_cast<unsigned int>(-1), static_cast<unsigned int>(Token::multiCompare("one|two", "ne")));
        ASSERT_EQUALS(static_cast<unsigned int>(-1), static_cast<unsigned int>(Token::multiCompare("abc|def", "a")));
        ASSERT_EQUALS(static_cast<unsigned int>(-1), static_cast<unsigned int>(Token::multiCompare("abc|def", "abcd")));
        ASSERT_EQUALS(static_cast<unsigned int>(-1), static_cast<unsigned int>(Token::multiCompare("abc|def", "default")));
    }

    void getStrLength()
    {
        Token *tok = new Token();

        tok->str("\"\"");
        ASSERT_EQUALS(0, Token::getStrLength(tok));

        tok->str("\"test\"");
        ASSERT_EQUALS(4, Token::getStrLength(tok));

        tok->str("\"test \\\\test\"");
        ASSERT_EQUALS(10, Token::getStrLength(tok));

        Tokenizer::deleteTokens(tok);
    }

};

REGISTER_TEST(TestTOKEN)
