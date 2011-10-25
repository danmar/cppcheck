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
#include "token.h"
#include "settings.h"

#include <cstring>

extern std::ostringstream errout;
class TestToken : public TestFixture {
public:
    TestToken() : TestFixture("TestToken")
    { }

private:

    void run() {
        TEST_CASE(nextprevious);
        TEST_CASE(multiCompare);
        TEST_CASE(getStrLength);
        TEST_CASE(strValue);

        TEST_CASE(deleteLast);
        TEST_CASE(nextArgument);

        TEST_CASE(matchAny);
        TEST_CASE(matchSingleChar);
        TEST_CASE(matchNothingOrAnyNotElse);
        TEST_CASE(matchNumeric);
        TEST_CASE(matchBoolean);
        TEST_CASE(matchOr);

        TEST_CASE(updateProperties)
        TEST_CASE(updatePropertiesConcatStr)
        TEST_CASE(isNameGuarantees1)
        TEST_CASE(isNameGuarantees2)
        TEST_CASE(isNameGuarantees3)
        TEST_CASE(isNameGuarantees4)
        TEST_CASE(isNameGuarantees5)
    }

    void nextprevious() {
        Token *token = new Token(0);
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

    void multiCompare() {
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

        // %op%
        ASSERT_EQUALS(1, Token::multiCompare("one|%op%", "+"));
        ASSERT_EQUALS(1, Token::multiCompare("%op%|two", "+"));
        ASSERT_EQUALS(-1, Token::multiCompare("one|%op%", "x"));
        ASSERT_EQUALS(-1, Token::multiCompare("%op%|two", "x"));
    }

    void getStrLength() {
        Token tok(0);

        tok.str("\"\"");
        ASSERT_EQUALS(0, (int)Token::getStrLength(&tok));

        tok.str("\"test\"");
        ASSERT_EQUALS(4, (int)Token::getStrLength(&tok));

        tok.str("\"test \\\\test\"");
        ASSERT_EQUALS(10, (int)Token::getStrLength(&tok));

        tok.str("\"a\\0\"");
        ASSERT_EQUALS(1, (int)Token::getStrLength(&tok));
    }

    void strValue() {
        Token tok(0);
        tok.str("\"\"");
        ASSERT_EQUALS(std::string(""), tok.strValue());

        tok.str("\"0\"");
        ASSERT_EQUALS(std::string("0"), tok.strValue());
    }


    void deleteLast() {
        Token *tokensBack = 0;
        Token tok(&tokensBack);
        tok.insertToken("aba");
        ASSERT_EQUALS(true, tokensBack == tok.next());
        tok.deleteNext();
        ASSERT_EQUALS(true, tokensBack == &tok);
    }

    void nextArgument() {
        givenACodeSampleToTokenize example1("foo(1, 2, 3, 4);");
        ASSERT_EQUALS(true, Token::Match(example1.tokens()->tokAt(2)->nextArgument(), "2 , 3"));

        givenACodeSampleToTokenize example2("foo();");
        ASSERT_EQUALS(true, example2.tokens()->tokAt(2)->nextArgument() == 0);

        givenACodeSampleToTokenize example3("foo(bar(a, b), 2, 3);");
        ASSERT_EQUALS(true, Token::Match(example3.tokens()->tokAt(2)->nextArgument(), "2 , 3"));
    }


    void matchAny() {
        givenACodeSampleToTokenize varBitOrVar("abc|def");
        ASSERT_EQUALS(true, Token::Match(varBitOrVar.tokens(), "%var% | %var%"));

        givenACodeSampleToTokenize varLogOrVar("abc||def");
        ASSERT_EQUALS(true, Token::Match(varLogOrVar.tokens(), "%var% || %var%"));
    }

    void matchSingleChar() {
        givenACodeSampleToTokenize singleChar("a");
        ASSERT_EQUALS(true, Token::Match(singleChar.tokens(), "[a|bc]"));
        ASSERT_EQUALS(false, Token::Match(singleChar.tokens(), "[d|ef]"));

        Token multiChar(0);
        multiChar.str("[ab");
        ASSERT_EQUALS(false, Token::Match(&multiChar, "[ab|def]"));
    }

    void matchNothingOrAnyNotElse() {
        givenACodeSampleToTokenize emptyString("");
        ASSERT_EQUALS(true, Token::Match(emptyString.tokens(), "!!else"));
        ASSERT_EQUALS(false, Token::Match(emptyString.tokens(), "!!else something"));

        givenACodeSampleToTokenize ifSemicolon("if ;");
        ASSERT_EQUALS(true, Token::Match(ifSemicolon.tokens(), "!!return if"));
        ASSERT_EQUALS(true, Token::Match(ifSemicolon.tokens(), "if ; !!else"));

        givenACodeSampleToTokenize ifSemicolonSomething("if ; something");
        ASSERT_EQUALS(true, Token::Match(ifSemicolonSomething.tokens(), "if ; !!else"));

        givenACodeSampleToTokenize justElse("else");
        ASSERT_EQUALS(false, Token::Match(justElse.tokens(), "!!else"));

        givenACodeSampleToTokenize ifSemicolonElse("if ; else");
        ASSERT_EQUALS(false, Token::Match(ifSemicolonElse.tokens(), "if ; !!else"));
    }


    void matchNumeric() {
        givenACodeSampleToTokenize nonNumeric("abc");
        ASSERT_EQUALS(false, Token::Match(nonNumeric.tokens(), "%num%"));

        givenACodeSampleToTokenize binary("101010b");
        ASSERT_EQUALS(true, Token::Match(binary.tokens(), "%num%"));

        givenACodeSampleToTokenize octal("0123");
        ASSERT_EQUALS(true, Token::Match(octal.tokens(), "%num%"));

        givenACodeSampleToTokenize decimal("4567");
        ASSERT_EQUALS(true, Token::Match(decimal.tokens(), "%num%"));

        givenACodeSampleToTokenize hexadecimal("0xDEADBEEF");
        ASSERT_EQUALS(true, Token::Match(hexadecimal.tokens(), "%num%"));

        givenACodeSampleToTokenize floatingPoint("0.0f");
        ASSERT_EQUALS(true, Token::Match(hexadecimal.tokens(), "%num%"));

        givenACodeSampleToTokenize doublePrecision("0.0d");
        ASSERT_EQUALS(true, Token::Match(hexadecimal.tokens(), "%num%"));

        givenACodeSampleToTokenize unsignedInt("0U");
        ASSERT_EQUALS(true, Token::Match(hexadecimal.tokens(), "%num%"));

        givenACodeSampleToTokenize unsignedLong("0UL");
        ASSERT_EQUALS(true, Token::Match(hexadecimal.tokens(), "%num%"));

        givenACodeSampleToTokenize unsignedLongLong("0ULL");
        ASSERT_EQUALS(true, Token::Match(hexadecimal.tokens(), "%num%"));

        givenACodeSampleToTokenize positive("+666");
        ASSERT_EQUALS(true, Token::Match(positive.tokens(), "+ %num%"));

        givenACodeSampleToTokenize negative("-42");
        ASSERT_EQUALS(true, Token::Match(negative.tokens(), "- %num%"));
    }


    void matchBoolean() {
        givenACodeSampleToTokenize yes("YES");
        ASSERT_EQUALS(false, Token::Match(yes.tokens(), "%bool%"));

        givenACodeSampleToTokenize positive("true");
        ASSERT_EQUALS(true, Token::Match(positive.tokens(), "%bool%"));

        givenACodeSampleToTokenize negative("false");
        ASSERT_EQUALS(true, Token::Match(negative.tokens(), "%bool%"));
    }

    void matchOr() {
        givenACodeSampleToTokenize bitwiseOr("|");
        ASSERT_EQUALS(true,  Token::Match(bitwiseOr.tokens(), "%or%"));
        ASSERT_EQUALS(false, Token::Match(bitwiseOr.tokens(), "%op%"));
        ASSERT_EQUALS(false, Token::Match(bitwiseOr.tokens(), "%oror%"));

        givenACodeSampleToTokenize logicalOr("||");
        ASSERT_EQUALS(false, Token::Match(logicalOr.tokens(), "%or%"));
        ASSERT_EQUALS(false, Token::Match(logicalOr.tokens(), "%op%"));
        ASSERT_EQUALS(true,  Token::Match(logicalOr.tokens(), "%oror%"));
        ASSERT_EQUALS(true,  Token::Match(logicalOr.tokens(), "&&|%oror%"));
        ASSERT_EQUALS(true,  Token::Match(logicalOr.tokens(), "%oror%|&&"));

        givenACodeSampleToTokenize logicalAnd("&&");
        ASSERT_EQUALS(true, Token::Match(logicalAnd.tokens(), "&&"));
        ASSERT_EQUALS(true, Token::Match(logicalAnd.tokens(), "&&|%oror%"));
        ASSERT_EQUALS(true, Token::Match(logicalAnd.tokens(), "%oror%|&&"));
    }

    void matchOp() {
        givenACodeSampleToTokenize op("+");
        ASSERT_EQUALS(true, Token::Match(op.tokens(), "%op%"));
    }

    void updateProperties() {
        Token tok(NULL);
        tok.str("foobar");

        ASSERT_EQUALS(true, tok.isName());
        ASSERT_EQUALS(false, tok.isNumber());

        tok.str("123456");

        ASSERT_EQUALS(false, tok.isName());
        ASSERT_EQUALS(true, tok.isNumber());
    }

    void updatePropertiesConcatStr() {
        Token tok(NULL);
        tok.str("true");

        ASSERT_EQUALS(true, tok.isBoolean());

        tok.concatStr("123");

        ASSERT_EQUALS(false, tok.isBoolean());
        ASSERT_EQUALS("tru23", tok.str());
    }

    void isNameGuarantees1() {
        Token tok(NULL);
        tok.str("Name");
        ASSERT_EQUALS(true, tok.isName());
    }

    void isNameGuarantees2() {
        Token tok(NULL);
        tok.str("_name");
        ASSERT_EQUALS(true, tok.isName());
    }

    void isNameGuarantees3() {
        Token tok(NULL);
        tok.str("_123");
        ASSERT_EQUALS(true, tok.isName());
    }

    void isNameGuarantees4() {
        Token tok(NULL);
        tok.str("123456");
        ASSERT_EQUALS(false, tok.isName());
        ASSERT_EQUALS(true, tok.isNumber());
    }

    void isNameGuarantees5() {
        Token tok(NULL);
        tok.str("a123456");
        ASSERT_EQUALS(true, tok.isName());
        ASSERT_EQUALS(false, tok.isNumber());
    }
};

REGISTER_TEST(TestToken)
