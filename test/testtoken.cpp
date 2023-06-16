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
#include "errortypes.h"
#include "fixture.h"
#include "helpers.h"
#include "token.h"
#include "tokenize.h"
#include "tokenlist.h"
#include "vfvalue.h"

#include <algorithm>
#include <sstream> // IWYU pragma: keep
#include <string>
#include <vector>


class TestToken : public TestFixture {
public:
    TestToken() : TestFixture("TestToken") {}

private:
    std::vector<std::string> arithmeticalOps;
    std::vector<std::string> logicalOps;
    std::vector<std::string> bitOps;
    std::vector<std::string> comparisonOps;
    std::vector<std::string> extendedOps;
    std::vector<std::string> assignmentOps;

    void run() override {
        arithmeticalOps = { "+", "-", "*", "/", "%", "<<", ">>" };
        logicalOps = { "&&", "||", "!" };
        comparisonOps = { "==", "!=", "<", "<=", ">", ">=" };
        bitOps = { "&", "|", "^", "~" };
        extendedOps = { ",", "[", "]", "(", ")", "?", ":" };
        assignmentOps = { "=", "+=", "-=", "*=", "/=", "%=", "&=", "^=", "|=", "<<=", ">>=" };

        TEST_CASE(nextprevious);
        TEST_CASE(multiCompare);
        TEST_CASE(multiCompare2);                   // #3294 - false negative multi compare between "=" and "=="
        TEST_CASE(multiCompare3);                   // false positive for %or% on code using "|="
        TEST_CASE(multiCompare4);
        TEST_CASE(multiCompare5);
        TEST_CASE(charTypes);
        TEST_CASE(stringTypes);
        TEST_CASE(getStrLength);
        TEST_CASE(getStrSize);
        TEST_CASE(strValue);
        TEST_CASE(concatStr);

        TEST_CASE(deleteLast);
        TEST_CASE(deleteFirst);
        TEST_CASE(nextArgument);
        TEST_CASE(eraseTokens);

        TEST_CASE(matchAny);
        TEST_CASE(matchSingleChar);
        TEST_CASE(matchNothingOrAnyNotElse);
        TEST_CASE(matchType);
        TEST_CASE(matchChar);
        TEST_CASE(matchCompOp);
        TEST_CASE(matchStr);
        TEST_CASE(matchVarid);
        TEST_CASE(matchNumeric);
        TEST_CASE(matchBoolean);
        TEST_CASE(matchOr);
        TEST_CASE(matchOp);
        TEST_CASE(matchConstOp);

        TEST_CASE(isArithmeticalOp);
        TEST_CASE(isOp);
        TEST_CASE(isConstOp);
        TEST_CASE(isExtendedOp);
        TEST_CASE(isAssignmentOp);
        TEST_CASE(isStandardType);
        TEST_CASE(literals);
        TEST_CASE(operators);

        TEST_CASE(updateProperties);
        TEST_CASE(isNameGuarantees1);
        TEST_CASE(isNameGuarantees2);
        TEST_CASE(isNameGuarantees3);
        TEST_CASE(isNameGuarantees4);
        TEST_CASE(isNameGuarantees5);
        TEST_CASE(isNameGuarantees6);

        TEST_CASE(canFindMatchingBracketsNeedsOpen);
        TEST_CASE(canFindMatchingBracketsInnerPair);
        TEST_CASE(canFindMatchingBracketsOuterPair);
        TEST_CASE(canFindMatchingBracketsWithTooManyClosing);
        TEST_CASE(canFindMatchingBracketsWithTooManyOpening);
        TEST_CASE(findClosingBracket);

        TEST_CASE(expressionString);

        TEST_CASE(hasKnownIntValue);
    }

    void nextprevious() const {
        Token *token = new Token();
        token->str("1");
        token->insertToken("2");
        token->next()->insertToken("3");
        Token *last = token->tokAt(2);
        ASSERT_EQUALS(token->str(), "1");
        ASSERT_EQUALS(token->next()->str(), "2");
        // cppcheck-suppress redundantNextPrevious - this is intentional
        ASSERT_EQUALS(token->tokAt(2)->str(), "3");
        ASSERT_EQUALS_MSG(true, last->next() == nullptr, "Null was expected");

        ASSERT_EQUALS(last->str(), "3");
        ASSERT_EQUALS(last->previous()->str(), "2");
        // cppcheck-suppress redundantNextPrevious - this is intentional
        ASSERT_EQUALS(last->tokAt(-2)->str(), "1");
        ASSERT_EQUALS_MSG(true, token->previous() == nullptr, "Null was expected");

        TokenList::deleteTokens(token);
    }

#define MatchCheck(...) MatchCheck_(__FILE__, __LINE__, __VA_ARGS__)
    bool MatchCheck_(const char* file, int line, const std::string& code, const std::string& pattern, unsigned int varid = 0) {
        const Settings settings;
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(";" + code + ";");
        try {
            ASSERT_LOC(tokenizer.tokenize(istr, "test.cpp"), file, line);
        } catch (...) {}
        return Token::Match(tokenizer.tokens()->next(), pattern.c_str(), varid);
    }

    void multiCompare() const {
        // Test for found
        Token one;
        one.str("one");
        ASSERT_EQUALS(1, Token::multiCompare(&one, "one|two", 0));

        Token two;
        two.str("two");
        ASSERT_EQUALS(1, Token::multiCompare(&two, "one|two", 0));
        ASSERT_EQUALS(1, Token::multiCompare(&two, "verybig|two|", 0));

        // Test for empty string found
        Token notfound;
        notfound.str("notfound");
        ASSERT_EQUALS(0, Token::multiCompare(&notfound, "one|two|", 0));

        // Test for not found
        ASSERT_EQUALS(static_cast<unsigned int>(-1), static_cast<unsigned int>(Token::multiCompare(&notfound, "one|two", 0)));

        Token s;
        s.str("s");
        ASSERT_EQUALS(static_cast<unsigned int>(-1), static_cast<unsigned int>(Token::multiCompare(&s, "verybig|two", 0)));

        Token ne;
        ne.str("ne");
        ASSERT_EQUALS(static_cast<unsigned int>(-1), static_cast<unsigned int>(Token::multiCompare(&ne, "one|two", 0)));

        Token a;
        a.str("a");
        ASSERT_EQUALS(static_cast<unsigned int>(-1), static_cast<unsigned int>(Token::multiCompare(&a, "abc|def", 0)));

        Token abcd;
        abcd.str("abcd");
        ASSERT_EQUALS(static_cast<unsigned int>(-1), static_cast<unsigned int>(Token::multiCompare(&abcd, "abc|def", 0)));

        Token def;
        def.str("default");
        ASSERT_EQUALS(static_cast<unsigned int>(-1), static_cast<unsigned int>(Token::multiCompare(&def, "abc|def", 0)));

        // %op%
        Token plus;
        plus.str("+");
        ASSERT_EQUALS(1, Token::multiCompare(&plus, "one|%op%", 0));
        ASSERT_EQUALS(1, Token::multiCompare(&plus, "%op%|two", 0));
        Token x;
        x.str("x");
        ASSERT_EQUALS(-1, Token::multiCompare(&x, "one|%op%", 0));
        ASSERT_EQUALS(-1, Token::multiCompare(&x, "%op%|two", 0));
    }

    void multiCompare2() const { // #3294
        // Original pattern that failed: [[,(=<>+-*|&^] %num% [+-*/] %num% ]|,|)|;|=|%op%
        givenACodeSampleToTokenize toks("a == 1", true);
        ASSERT_EQUALS(true, Token::Match(toks.tokens(), "a =|%op%"));
    }

    void multiCompare3() const {
        // Original pattern that failed: "return|(|&&|%oror% %name% &&|%oror%|==|!=|<=|>=|<|>|-|%or% %name% )|&&|%oror%|;"
        // Code snippet that failed: "return lv@86 |= rv@87 ;"

        // Note: Also test "reverse" alternative pattern, two different code paths to handle it
        givenACodeSampleToTokenize toks("return a |= b ;", true);
        ASSERT_EQUALS(false, Token::Match(toks.tokens(), "return %name% xyz|%or% %name% ;"));
        ASSERT_EQUALS(false, Token::Match(toks.tokens(), "return %name% %or%|xyz %name% ;"));

        givenACodeSampleToTokenize toks2("return a | b ;", true);
        ASSERT_EQUALS(true, Token::Match(toks2.tokens(), "return %name% xyz|%or% %name% ;"));
        ASSERT_EQUALS(true, Token::Match(toks2.tokens(), "return %name% %or%|xyz %name% ;"));

        givenACodeSampleToTokenize toks3("return a || b ;", true);
        ASSERT_EQUALS(false, Token::Match(toks3.tokens(), "return %name% xyz|%or% %name% ;"));
        ASSERT_EQUALS(false, Token::Match(toks3.tokens(), "return %name% %or%|xyz %name% ;"));

        ASSERT_EQUALS(true, Token::Match(toks3.tokens(), "return %name% xyz|%oror% %name% ;"));
        ASSERT_EQUALS(true, Token::Match(toks3.tokens(), "return %name% %oror%|xyz %name% ;"));

        givenACodeSampleToTokenize toks4("a % b ;", true);
        ASSERT_EQUALS(true, Token::Match(toks4.tokens(), "%name% >>|<<|&|%or%|^|% %name% ;"));
        ASSERT_EQUALS(true, Token::Match(toks4.tokens(), "%name% %|>>|<<|&|%or%|^ %name% ;"));
        ASSERT_EQUALS(true, Token::Match(toks4.tokens(), "%name% >>|<<|&|%or%|%|^ %name% ;"));

        //%name%|%num% support
        givenACodeSampleToTokenize num("100", true);
        ASSERT_EQUALS(true, Token::Match(num.tokens(), "%num%|%name%"));
        ASSERT_EQUALS(true, Token::Match(num.tokens(), "%name%|%num%"));
        ASSERT_EQUALS(true, Token::Match(num.tokens(), "%name%|%num%|%bool%"));
        ASSERT_EQUALS(true, Token::Match(num.tokens(), "%name%|%bool%|%num%"));
        ASSERT_EQUALS(true, Token::Match(num.tokens(), "%name%|%bool%|%str%|%num%"));
        ASSERT_EQUALS(false, Token::Match(num.tokens(), "%bool%|%name%"));
        ASSERT_EQUALS(false, Token::Match(num.tokens(), "%type%|%bool%|%char%"));
        ASSERT_EQUALS(true, Token::Match(num.tokens(), "%type%|%bool%|100"));

        givenACodeSampleToTokenize numparen("( 100 )", true);
        ASSERT_EQUALS(true, Token::Match(numparen.tokens(), "(| %num%|%name% )|"));
        ASSERT_EQUALS(true, Token::Match(numparen.tokens(), "(| %name%|%num% )|"));
        ASSERT_EQUALS(true, Token::Match(numparen.tokens(), "(| %name%|%num%|%bool% )|"));
        ASSERT_EQUALS(true, Token::Match(numparen.tokens(), "(| %name%|%bool%|%num% )|"));
        ASSERT_EQUALS(true, Token::Match(numparen.tokens(), "(| %name%|%bool%|%str%|%num% )|"));
        ASSERT_EQUALS(false, Token::Match(numparen.tokens(), "(| %bool%|%name% )|"));

        ASSERT_EQUALS(true, Token::Match(numparen.tokens(), "(| 100 %num%|%name%| )|"));
        ASSERT_EQUALS(true, Token::Match(numparen.tokens(), "(| 100 %name%|%num%| )|"));
        ASSERT_EQUALS(true, Token::Match(numparen.tokens(), "(| 100 %name%|%num%|%bool%| )|"));
        ASSERT_EQUALS(true, Token::Match(numparen.tokens(), "(| 100 %name%|%bool%|%num%| )|"));
        ASSERT_EQUALS(true, Token::Match(numparen.tokens(), "(| 100 %name%|%bool%|%str%|%num%| )|"));
        ASSERT_EQUALS(true, Token::Match(numparen.tokens(), "(| 100 %bool%|%name%| )|"));
    }

    void multiCompare4() const {
        givenACodeSampleToTokenize var("std :: queue < int > foo ;");

        ASSERT_EQUALS(Token::eBracket, var.tokens()->tokAt(3)->tokType());
        ASSERT_EQUALS(Token::eBracket, var.tokens()->tokAt(5)->tokType());

        ASSERT_EQUALS(false, Token::Match(var.tokens(), "std :: queue %op%"));
        ASSERT_EQUALS(false, Token::Match(var.tokens(), "std :: queue x|%op%"));
        ASSERT_EQUALS(false, Token::Match(var.tokens(), "std :: queue %op%|x"));
    }

    void multiCompare5() const {
        Token tok;
        tok.str("||");
        ASSERT_EQUALS(true, Token::multiCompare(&tok, "+|%or%|%oror%", 0) >= 0);
    }

    void charTypes() const {
        Token tok;

        tok.str("'a'");
        ASSERT_EQUALS(true, tok.isCChar());
        ASSERT_EQUALS(false, tok.isUtf8());
        ASSERT_EQUALS(false, tok.isUtf16());
        ASSERT_EQUALS(false, tok.isUtf32());
        ASSERT_EQUALS(false, tok.isLong());
        ASSERT_EQUALS(false, tok.isCMultiChar());

        tok.str("u8'a'");
        ASSERT_EQUALS(false, tok.isCChar());
        ASSERT_EQUALS(true, tok.isUtf8());
        ASSERT_EQUALS(false, tok.isUtf16());
        ASSERT_EQUALS(false, tok.isUtf32());
        ASSERT_EQUALS(false, tok.isLong());
        ASSERT_EQUALS(false, tok.isCMultiChar());

        tok.str("u'a'");
        ASSERT_EQUALS(false, tok.isCChar());
        ASSERT_EQUALS(false, tok.isUtf8());
        ASSERT_EQUALS(true, tok.isUtf16());
        ASSERT_EQUALS(false, tok.isUtf32());
        ASSERT_EQUALS(false, tok.isLong());
        ASSERT_EQUALS(false, tok.isCMultiChar());

        tok.str("U'a'");
        ASSERT_EQUALS(false, tok.isCChar());
        ASSERT_EQUALS(false, tok.isUtf8());
        ASSERT_EQUALS(false, tok.isUtf16());
        ASSERT_EQUALS(true, tok.isUtf32());
        ASSERT_EQUALS(false, tok.isLong());
        ASSERT_EQUALS(false, tok.isCMultiChar());

        tok.str("L'a'");
        ASSERT_EQUALS(false, tok.isCChar());
        ASSERT_EQUALS(false, tok.isUtf8());
        ASSERT_EQUALS(false, tok.isUtf16());
        ASSERT_EQUALS(false, tok.isUtf32());
        ASSERT_EQUALS(true, tok.isLong());
        ASSERT_EQUALS(false, tok.isCMultiChar());

        tok.str("'aaa'");
        ASSERT_EQUALS(false, tok.isCChar());
        ASSERT_EQUALS(false, tok.isUtf8());
        ASSERT_EQUALS(false, tok.isUtf16());
        ASSERT_EQUALS(false, tok.isUtf32());
        ASSERT_EQUALS(false, tok.isLong());
        ASSERT_EQUALS(true, tok.isCMultiChar());
    }

    void stringTypes() const {
        Token tok;

        tok.str("\"a\"");
        ASSERT_EQUALS(true, tok.isCChar());
        ASSERT_EQUALS(false, tok.isUtf8());
        ASSERT_EQUALS(false, tok.isUtf16());
        ASSERT_EQUALS(false, tok.isUtf32());
        ASSERT_EQUALS(false, tok.isLong());
        ASSERT_EQUALS(false, tok.isCMultiChar());

        tok.str("u8\"a\"");
        ASSERT_EQUALS(false, tok.isCChar());
        ASSERT_EQUALS(true, tok.isUtf8());
        ASSERT_EQUALS(false, tok.isUtf16());
        ASSERT_EQUALS(false, tok.isUtf32());
        ASSERT_EQUALS(false, tok.isLong());
        ASSERT_EQUALS(false, tok.isCMultiChar());

        tok.str("u\"a\"");
        ASSERT_EQUALS(false, tok.isCChar());
        ASSERT_EQUALS(false, tok.isUtf8());
        ASSERT_EQUALS(true, tok.isUtf16());
        ASSERT_EQUALS(false, tok.isUtf32());
        ASSERT_EQUALS(false, tok.isLong());
        ASSERT_EQUALS(false, tok.isCMultiChar());

        tok.str("U\"a\"");
        ASSERT_EQUALS(false, tok.isCChar());
        ASSERT_EQUALS(false, tok.isUtf8());
        ASSERT_EQUALS(false, tok.isUtf16());
        ASSERT_EQUALS(true, tok.isUtf32());
        ASSERT_EQUALS(false, tok.isLong());
        ASSERT_EQUALS(false, tok.isCMultiChar());

        tok.str("L\"a\"");
        ASSERT_EQUALS(false, tok.isCChar());
        ASSERT_EQUALS(false, tok.isUtf8());
        ASSERT_EQUALS(false, tok.isUtf16());
        ASSERT_EQUALS(false, tok.isUtf32());
        ASSERT_EQUALS(true, tok.isLong());
        ASSERT_EQUALS(false, tok.isCMultiChar());
    }

    void getStrLength() const {
        Token tok;

        tok.str("\"\"");
        ASSERT_EQUALS(0, Token::getStrLength(&tok));

        tok.str("\"test\"");
        ASSERT_EQUALS(4, Token::getStrLength(&tok));

        tok.str("\"test \\\\test\"");
        ASSERT_EQUALS(10, Token::getStrLength(&tok));

        tok.str("\"a\\0\"");
        ASSERT_EQUALS(1, Token::getStrLength(&tok));

        tok.str("L\"\"");
        ASSERT_EQUALS(0, Token::getStrLength(&tok));

        tok.str("u8\"test\"");
        ASSERT_EQUALS(4, Token::getStrLength(&tok));

        tok.str("U\"test \\\\test\"");
        ASSERT_EQUALS(10, Token::getStrLength(&tok));

        tok.str("u\"a\\0\"");
        ASSERT_EQUALS(1, Token::getStrLength(&tok));
    }

    void getStrSize() const {
        Token tok;
        const Settings settings;

        tok.str("\"\"");
        ASSERT_EQUALS(sizeof(""), Token::getStrSize(&tok, &settings));

        tok.str("\"abc\"");
        ASSERT_EQUALS(sizeof("abc"), Token::getStrSize(&tok, &settings));

        tok.str("\"\\0abc\"");
        ASSERT_EQUALS(sizeof("\0abc"), Token::getStrSize(&tok, &settings));

        tok.str("\"\\\\\"");
        ASSERT_EQUALS(sizeof("\\"), Token::getStrSize(&tok, &settings));
    }

    void strValue() const {
        Token tok;

        tok.str("\"\"");
        ASSERT_EQUALS("", tok.strValue());

        tok.str("\"0\"");
        ASSERT_EQUALS("0", tok.strValue());

        tok.str("\"a\\n\"");
        ASSERT_EQUALS("a\n", tok.strValue());

        tok.str("\"a\\r\"");
        ASSERT_EQUALS("a\r", tok.strValue());

        tok.str("\"a\\t\"");
        ASSERT_EQUALS("a\t", tok.strValue());

        tok.str("\"\\\\\"");
        ASSERT_EQUALS("\\", tok.strValue());

        tok.str("\"a\\0\"");
        ASSERT_EQUALS("a", tok.strValue());

        tok.str("L\"a\\t\"");
        ASSERT_EQUALS("a\t", tok.strValue());

        tok.str("U\"a\\0\"");
        ASSERT_EQUALS("a", tok.strValue());
    }

    void concatStr() const {
        Token tok;

        tok.str("\"\"");
        tok.concatStr("\"\"");
        ASSERT_EQUALS("", tok.strValue());
        ASSERT(tok.isCChar());

        tok.str("\"ab\"");
        tok.concatStr("\"cd\"");
        ASSERT_EQUALS("abcd", tok.strValue());
        ASSERT(tok.isCChar());

        tok.str("L\"ab\"");
        tok.concatStr("L\"cd\"");
        ASSERT_EQUALS("abcd", tok.strValue());
        ASSERT(tok.isLong());

        tok.str("L\"ab\"");
        tok.concatStr("\"cd\"");
        ASSERT_EQUALS("abcd", tok.strValue());
        ASSERT(tok.isLong());

        tok.str("\"ab\"");
        tok.concatStr("L\"cd\"");
        ASSERT_EQUALS("abcd", tok.strValue());
        ASSERT(tok.isLong());

        tok.str("\"ab\"");
        tok.concatStr("L\"\"");
        ASSERT_EQUALS("ab", tok.strValue());
        ASSERT(tok.isLong());

        tok.str("\"ab\"");
        tok.concatStr("u8\"cd\"");
        ASSERT_EQUALS("abcd", tok.strValue());
        ASSERT(tok.isUtf8());
    }

    void deleteLast() const {
        TokensFrontBack listEnds;
        Token ** const tokensBack = &(listEnds.back);
        Token tok(&listEnds);
        tok.insertToken("aba");
        ASSERT_EQUALS(true, *tokensBack == tok.next());
        tok.deleteNext();
        ASSERT_EQUALS(true, *tokensBack == &tok);
    }

    void deleteFirst() const {
        TokensFrontBack listEnds;
        Token ** const tokensFront = &(listEnds.front);
        Token tok(&listEnds);

        tok.insertToken("aba");

        ASSERT_EQUALS(true, *tokensFront == tok.previous());
        tok.deletePrevious();
        ASSERT_EQUALS(true, *tokensFront == &tok);
    }

    void nextArgument() const {
        givenACodeSampleToTokenize example1("foo(1, 2, 3, 4);");
        ASSERT_EQUALS(true, Token::simpleMatch(example1.tokens()->tokAt(2)->nextArgument(), "2 , 3"));
        ASSERT_EQUALS(true, Token::simpleMatch(example1.tokens()->tokAt(4)->nextArgument(), "3 , 4"));

        givenACodeSampleToTokenize example2("foo();");
        ASSERT_EQUALS(true, example2.tokens()->tokAt(2)->nextArgument() == nullptr);

        givenACodeSampleToTokenize example3("foo(bar(a, b), 2, 3);");
        ASSERT_EQUALS(true, Token::simpleMatch(example3.tokens()->tokAt(2)->nextArgument(), "2 , 3"));

        givenACodeSampleToTokenize example4("foo(x.i[1], \"\", 3);");
        ASSERT_EQUALS(true, Token::simpleMatch(example4.tokens()->tokAt(2)->nextArgument(), "\"\" , 3"));
    }

    void eraseTokens() const {
        givenACodeSampleToTokenize code("begin ; { this code will be removed } end", true);
        Token::eraseTokens(code.tokens()->next(), code.tokens()->tokAt(9));
        ASSERT_EQUALS("begin ; end", code.tokens()->stringifyList(nullptr, false));
    }


    void matchAny() const {
        givenACodeSampleToTokenize varBitOrVar("abc|def", true);
        ASSERT_EQUALS(true, Token::Match(varBitOrVar.tokens(), "%name% %or% %name%"));

        givenACodeSampleToTokenize varLogOrVar("abc||def", true);
        ASSERT_EQUALS(true, Token::Match(varLogOrVar.tokens(), "%name% %oror% %name%"));
    }

    void matchSingleChar() const {
        givenACodeSampleToTokenize singleChar("a", true);
        ASSERT_EQUALS(true, Token::Match(singleChar.tokens(), "[a|bc]"));
        ASSERT_EQUALS(false, Token::Match(singleChar.tokens(), "[d|ef]"));

        Token multiChar;
        multiChar.str("[ab");
        ASSERT_EQUALS(false, Token::Match(&multiChar, "[ab|def]"));
    }

    void matchNothingOrAnyNotElse() const {
        givenACodeSampleToTokenize empty_String("", true);
        ASSERT_EQUALS(true, Token::Match(empty_String.tokens(), "!!else"));
        ASSERT_EQUALS(false, Token::Match(empty_String.tokens(), "!!else something"));

        givenACodeSampleToTokenize ifSemicolon("if ;", true);
        ASSERT_EQUALS(true, Token::Match(ifSemicolon.tokens(), "if ; !!else"));

        givenACodeSampleToTokenize ifSemicolonSomething("if ; something", true);
        ASSERT_EQUALS(true, Token::Match(ifSemicolonSomething.tokens(), "if ; !!else"));

        givenACodeSampleToTokenize justElse("else", true);
        ASSERT_EQUALS(false, Token::Match(justElse.tokens(), "!!else"));

        givenACodeSampleToTokenize ifSemicolonElse("if ; else", true);
        ASSERT_EQUALS(false, Token::Match(ifSemicolonElse.tokens(), "if ; !!else"));
    }

    void matchType() const {
        givenACodeSampleToTokenize type("abc", true);
        ASSERT_EQUALS(true, Token::Match(type.tokens(), "%type%"));

        givenACodeSampleToTokenize isVar("int a = 3 ;");
        ASSERT_EQUALS(true, Token::Match(isVar.tokens(), "%type%"));
        ASSERT_EQUALS(true, Token::Match(isVar.tokens(), "%type% %name%"));
        ASSERT_EQUALS(false, Token::Match(isVar.tokens(), "%type% %type%"));

        givenACodeSampleToTokenize noType1_cpp("delete", true, true);
        ASSERT_EQUALS(false, Token::Match(noType1_cpp.tokens(), "%type%"));

        givenACodeSampleToTokenize noType1_c("delete", true, false);
        ASSERT_EQUALS(true, Token::Match(noType1_c.tokens(), "%type%"));

        givenACodeSampleToTokenize noType2("void delete", true);
        ASSERT_EQUALS(false, Token::Match(noType2.tokens(), "!!foo %type%"));
    }

    void matchChar() const {
        givenACodeSampleToTokenize chr1("'a'", true);
        ASSERT_EQUALS(true, Token::Match(chr1.tokens(), "%char%"));

        givenACodeSampleToTokenize chr2("'1'", true);
        ASSERT_EQUALS(true, Token::Match(chr2.tokens(), "%char%"));

        givenACodeSampleToTokenize noChr("\"10\"", true);
        ASSERT_EQUALS(false, Token::Match(noChr.tokens(), "%char%"));
    }

    void matchCompOp() const {
        givenACodeSampleToTokenize comp1("<=", true);
        ASSERT_EQUALS(true, Token::Match(comp1.tokens(), "%comp%"));

        givenACodeSampleToTokenize comp2(">", true);
        ASSERT_EQUALS(true, Token::Match(comp2.tokens(), "%comp%"));

        givenACodeSampleToTokenize noComp("=", true);
        ASSERT_EQUALS(false, Token::Match(noComp.tokens(), "%comp%"));
    }

    void matchStr() const {
        givenACodeSampleToTokenize noStr1("abc", true);
        ASSERT_EQUALS(false, Token::Match(noStr1.tokens(), "%str%"));

        givenACodeSampleToTokenize noStr2("'a'", true);
        ASSERT_EQUALS(false, Token::Match(noStr2.tokens(), "%str%"));

        givenACodeSampleToTokenize str("\"abc\"", true);
        ASSERT_EQUALS(true, Token::Match(str.tokens(), "%str%"));

        // Empty string
        givenACodeSampleToTokenize emptyStr("\"\"", true);
        ASSERT_EQUALS(true, Token::Match(emptyStr.tokens(), "%str%"));
    }

    void matchVarid() const {
        givenACodeSampleToTokenize var("int a ; int b ;");

        // Varid == 0 should throw exception
        ASSERT_THROW((void)Token::Match(var.tokens(), "%type% %varid% ; %type% %name%", 0),InternalError);

        ASSERT_EQUALS(true, Token::Match(var.tokens(), "%type% %varid% ; %type% %name%", 1));
        ASSERT_EQUALS(true, Token::Match(var.tokens(), "%type% %name% ; %type% %varid%", 2));

        // Try to match two different varids in one match call
        ASSERT_EQUALS(false, Token::Match(var.tokens(), "%type% %varid% ; %type% %varid%", 2));

        // %var% matches with every varid other than 0
        ASSERT_EQUALS(true, Token::Match(var.tokens(), "%type% %var% ;"));
        ASSERT_EQUALS(false, Token::Match(var.tokens(), "%var% %var% ;"));
    }

    void matchNumeric() const {
        givenACodeSampleToTokenize nonNumeric("abc", true);
        ASSERT_EQUALS(false, Token::Match(nonNumeric.tokens(), "%num%"));

        givenACodeSampleToTokenize binary("101010b", true);
        ASSERT_EQUALS(true, Token::Match(binary.tokens(), "%num%"));

        givenACodeSampleToTokenize octal("0123", true);
        ASSERT_EQUALS(true, Token::Match(octal.tokens(), "%num%"));

        givenACodeSampleToTokenize decimal("4567", true);
        ASSERT_EQUALS(true, Token::Match(decimal.tokens(), "%num%"));

        givenACodeSampleToTokenize hexadecimal("0xDEADBEEF", true);
        ASSERT_EQUALS(true, Token::Match(hexadecimal.tokens(), "%num%"));

        givenACodeSampleToTokenize floatingPoint("0.0f", true);
        ASSERT_EQUALS(true, Token::Match(floatingPoint.tokens(), "%num%"));

        givenACodeSampleToTokenize doublePrecision("0.0d", true);
        ASSERT_EQUALS(true, Token::Match(doublePrecision.tokens(), "%num%"));

        givenACodeSampleToTokenize signedLong("0L", true);
        ASSERT_EQUALS(true, Token::Match(signedLong.tokens(), "%num%"));

        givenACodeSampleToTokenize negativeSignedLong("-0L", true);
        ASSERT_EQUALS(true, Token::Match(negativeSignedLong.tokens(), "- %num%"));

        givenACodeSampleToTokenize positiveSignedLong("+0L", true);
        ASSERT_EQUALS(true, Token::Match(positiveSignedLong.tokens(), "+ %num%"));

        givenACodeSampleToTokenize unsignedInt("0U", true);
        ASSERT_EQUALS(true, Token::Match(unsignedInt.tokens(), "%num%"));

        givenACodeSampleToTokenize unsignedLong("0UL", true);
        ASSERT_EQUALS(true, Token::Match(unsignedLong.tokens(), "%num%"));

        givenACodeSampleToTokenize unsignedLongLong("0ULL", true);
        ASSERT_EQUALS(true, Token::Match(unsignedLongLong.tokens(), "%num%"));

        givenACodeSampleToTokenize positive("+666", true);
        ASSERT_EQUALS(true, Token::Match(positive.tokens(), "+ %num%"));

        givenACodeSampleToTokenize negative("-42", true);
        ASSERT_EQUALS(true, Token::Match(negative.tokens(), "- %num%"));

        givenACodeSampleToTokenize negativeNull("-.0", true);
        ASSERT_EQUALS(true, Token::Match(negativeNull.tokens(), "- %num%"));

        givenACodeSampleToTokenize positiveNull("+.0", true);
        ASSERT_EQUALS(true, Token::Match(positiveNull.tokens(), "+ %num%"));
    }


    void matchBoolean() const {
        givenACodeSampleToTokenize yes("YES", true);
        ASSERT_EQUALS(false, Token::Match(yes.tokens(), "%bool%"));

        givenACodeSampleToTokenize positive("true", true);
        ASSERT_EQUALS(true, Token::Match(positive.tokens(), "%bool%"));

        givenACodeSampleToTokenize negative("false", true);
        ASSERT_EQUALS(true, Token::Match(negative.tokens(), "%bool%"));
    }

    void matchOr() const {
        givenACodeSampleToTokenize bitwiseOr(";|;", true);
        // cppcheck-suppress simplePatternError - this is intentional
        ASSERT_EQUALS(true,  Token::Match(bitwiseOr.tokens(), "; %or%"));
        ASSERT_EQUALS(true,  Token::Match(bitwiseOr.tokens(), "; %op%"));
        // cppcheck-suppress simplePatternError - this is intentional
        ASSERT_EQUALS(false, Token::Match(bitwiseOr.tokens(), "; %oror%"));

        givenACodeSampleToTokenize bitwiseOrAssignment(";|=;");
        // cppcheck-suppress simplePatternError - this is intentional
        ASSERT_EQUALS(false,  Token::Match(bitwiseOrAssignment.tokens(), "; %or%"));
        ASSERT_EQUALS(true,  Token::Match(bitwiseOrAssignment.tokens(), "; %op%"));
        // cppcheck-suppress simplePatternError - this is intentional
        ASSERT_EQUALS(false, Token::Match(bitwiseOrAssignment.tokens(), "; %oror%"));

        givenACodeSampleToTokenize logicalOr(";||;", true);
        // cppcheck-suppress simplePatternError - this is intentional
        ASSERT_EQUALS(false, Token::Match(logicalOr.tokens(), "; %or%"));
        ASSERT_EQUALS(true,  Token::Match(logicalOr.tokens(), "; %op%"));
        // cppcheck-suppress simplePatternError - this is intentional
        ASSERT_EQUALS(true,  Token::Match(logicalOr.tokens(), "; %oror%"));
        ASSERT_EQUALS(true,  Token::Match(logicalOr.tokens(), "; &&|%oror%"));
        ASSERT_EQUALS(true,  Token::Match(logicalOr.tokens(), "; %oror%|&&"));

        givenACodeSampleToTokenize logicalAnd(";&&;", true);
        ASSERT_EQUALS(true, Token::simpleMatch(logicalAnd.tokens(), "; &&"));
        ASSERT_EQUALS(true, Token::Match(logicalAnd.tokens(), "; &&|%oror%"));
        ASSERT_EQUALS(true, Token::Match(logicalAnd.tokens(), "; %oror%|&&"));
    }

    static void append_vector(std::vector<std::string> &dest, const std::vector<std::string> &src) {
        dest.insert(dest.end(), src.cbegin(), src.cend());
    }

    void matchOp() {
        std::vector<std::string> test_ops;
        append_vector(test_ops, arithmeticalOps);
        append_vector(test_ops, bitOps);
        append_vector(test_ops, comparisonOps);
        append_vector(test_ops, logicalOps);
        append_vector(test_ops, assignmentOps);

        ASSERT_EQUALS(true, std::all_of(test_ops.cbegin(), test_ops.cend(), [&](const std::string& s) {
            return MatchCheck(s, "%op%");
        }));

        // Negative test against other operators
        std::vector<std::string> other_ops;
        append_vector(other_ops, extendedOps);

        std::vector<std::string>::const_iterator other_op, other_ops_end = other_ops.cend();
        for (other_op = other_ops.cbegin(); other_op != other_ops_end; ++other_op) {
            ASSERT_EQUALS_MSG(false, MatchCheck(*other_op, "%op%"), "Failing other operator: " + *other_op);
        }
    }

    void matchConstOp() {
        std::vector<std::string> test_ops;
        append_vector(test_ops, arithmeticalOps);
        append_vector(test_ops, bitOps);
        append_vector(test_ops, comparisonOps);
        append_vector(test_ops, logicalOps);

        ASSERT_EQUALS(true, std::all_of(test_ops.cbegin(), test_ops.cend(), [&](const std::string& s) {
            return MatchCheck(s, "%cop%");
        }));

        // Negative test against other operators
        std::vector<std::string> other_ops;
        append_vector(other_ops, extendedOps);
        append_vector(other_ops, assignmentOps);

        std::vector<std::string>::const_iterator other_op, other_ops_end = other_ops.cend();
        for (other_op = other_ops.cbegin(); other_op != other_ops_end; ++other_op) {
            ASSERT_EQUALS_MSG(false, MatchCheck(*other_op, "%cop%"), "Failing other operator: " + *other_op);
        }
    }


    void isArithmeticalOp() const {
        std::vector<std::string>::const_iterator test_op, test_ops_end = arithmeticalOps.cend();
        for (test_op = arithmeticalOps.cbegin(); test_op != test_ops_end; ++test_op) {
            Token tok;
            tok.str(*test_op);
            ASSERT_EQUALS(true, tok.isArithmeticalOp());
        }

        // Negative test against other operators
        std::vector<std::string> other_ops;
        append_vector(other_ops, bitOps);
        append_vector(other_ops, comparisonOps);
        append_vector(other_ops, logicalOps);
        append_vector(other_ops, extendedOps);
        append_vector(other_ops, assignmentOps);

        std::vector<std::string>::const_iterator other_op, other_ops_end = other_ops.cend();
        for (other_op = other_ops.cbegin(); other_op != other_ops_end; ++other_op) {
            Token tok;
            tok.str(*other_op);
            ASSERT_EQUALS_MSG(false, tok.isArithmeticalOp(), "Failing arithmetical operator: " + *other_op);
        }
    }

    void isOp() const {
        std::vector<std::string> test_ops;
        append_vector(test_ops, arithmeticalOps);
        append_vector(test_ops, bitOps);
        append_vector(test_ops, comparisonOps);
        append_vector(test_ops, logicalOps);
        append_vector(test_ops, assignmentOps);

        std::vector<std::string>::const_iterator test_op, test_ops_end = test_ops.cend();
        for (test_op = test_ops.cbegin(); test_op != test_ops_end; ++test_op) {
            Token tok;
            tok.str(*test_op);
            ASSERT_EQUALS(true, tok.isOp());
        }

        // Negative test against other operators
        std::vector<std::string> other_ops;
        append_vector(other_ops, extendedOps);

        std::vector<std::string>::const_iterator other_op, other_ops_end = other_ops.cend();
        for (other_op = other_ops.cbegin(); other_op != other_ops_end; ++other_op) {
            Token tok;
            tok.str(*other_op);
            ASSERT_EQUALS_MSG(false, tok.isOp(), "Failing normal operator: " + *other_op);
        }
    }

    void isConstOp() const {
        std::vector<std::string> test_ops;
        append_vector(test_ops, arithmeticalOps);
        append_vector(test_ops, bitOps);
        append_vector(test_ops, comparisonOps);
        append_vector(test_ops, logicalOps);

        std::vector<std::string>::const_iterator test_op, test_ops_end = test_ops.cend();
        for (test_op = test_ops.cbegin(); test_op != test_ops_end; ++test_op) {
            Token tok;
            tok.str(*test_op);
            ASSERT_EQUALS(true, tok.isConstOp());
        }

        // Negative test against other operators
        std::vector<std::string> other_ops;
        append_vector(other_ops, extendedOps);
        append_vector(other_ops, assignmentOps);

        std::vector<std::string>::const_iterator other_op, other_ops_end = other_ops.cend();
        for (other_op = other_ops.cbegin(); other_op != other_ops_end; ++other_op) {
            Token tok;
            tok.str(*other_op);
            ASSERT_EQUALS_MSG(false, tok.isConstOp(), "Failing normal operator: " + *other_op);
        }
    }

    void isExtendedOp() const {
        std::vector<std::string> test_ops;
        append_vector(test_ops, arithmeticalOps);
        append_vector(test_ops, bitOps);
        append_vector(test_ops, comparisonOps);
        append_vector(test_ops, logicalOps);
        append_vector(test_ops, extendedOps);

        std::vector<std::string>::const_iterator test_op, test_ops_end = test_ops.cend();
        for (test_op = test_ops.cbegin(); test_op != test_ops_end; ++test_op) {
            Token tok;
            tok.str(*test_op);
            ASSERT_EQUALS(true, tok.isExtendedOp());
        }

        // Negative test against assignment operators
        std::vector<std::string>::const_iterator other_op, other_ops_end = assignmentOps.cend();
        for (other_op = assignmentOps.cbegin(); other_op != other_ops_end; ++other_op) {
            Token tok;
            tok.str(*other_op);
            ASSERT_EQUALS_MSG(false, tok.isExtendedOp(), "Failing assignment operator: " + *other_op);
        }
    }

    void isAssignmentOp() const {
        std::vector<std::string>::const_iterator test_op, test_ops_end = assignmentOps.cend();
        for (test_op = assignmentOps.cbegin(); test_op != test_ops_end; ++test_op) {
            Token tok;
            tok.str(*test_op);
            ASSERT_EQUALS(true, tok.isAssignmentOp());
        }

        // Negative test against other operators
        std::vector<std::string> other_ops;
        append_vector(other_ops, arithmeticalOps);
        append_vector(other_ops, bitOps);
        append_vector(other_ops, comparisonOps);
        append_vector(other_ops, logicalOps);
        append_vector(other_ops, extendedOps);

        std::vector<std::string>::const_iterator other_op, other_ops_end = other_ops.cend();
        for (other_op = other_ops.cbegin(); other_op != other_ops_end; ++other_op) {
            Token tok;
            tok.str(*other_op);
            ASSERT_EQUALS_MSG(false, tok.isAssignmentOp(), "Failing assignment operator: " + *other_op);
        }
    }

    void operators() const {
        std::vector<std::string>::const_iterator test_op;
        for (test_op = extendedOps.cbegin(); test_op != extendedOps.cend(); ++test_op) {
            Token tok;
            tok.str(*test_op);
            ASSERT_EQUALS(Token::eExtendedOp, tok.tokType());
        }
        for (test_op = logicalOps.cbegin(); test_op != logicalOps.cend(); ++test_op) {
            Token tok;
            tok.str(*test_op);
            ASSERT_EQUALS(Token::eLogicalOp, tok.tokType());
        }
        for (test_op = bitOps.cbegin(); test_op != bitOps.cend(); ++test_op) {
            Token tok;
            tok.str(*test_op);
            ASSERT_EQUALS(Token::eBitOp, tok.tokType());
        }
        for (test_op = comparisonOps.cbegin(); test_op != comparisonOps.cend(); ++test_op) {
            Token tok;
            tok.str(*test_op);
            ASSERT_EQUALS(Token::eComparisonOp, tok.tokType());
        }
        Token tok;
        tok.str("++");
        ASSERT_EQUALS(Token::eIncDecOp, tok.tokType());
        tok.str("--");
        ASSERT_EQUALS(Token::eIncDecOp, tok.tokType());
    }

    void literals() const {
        Token tok;

        tok.str("\"foo\"");
        ASSERT(tok.tokType() == Token::eString);
        tok.str("\"\"");
        ASSERT(tok.tokType() == Token::eString);
        tok.str("'f'");
        ASSERT(tok.tokType() == Token::eChar);
        tok.str("12345");
        ASSERT(tok.tokType() == Token::eNumber);
        tok.str("-55");
        ASSERT(tok.tokType() == Token::eNumber);
        tok.str("true");
        ASSERT(tok.tokType() == Token::eBoolean);
        tok.str("false");
        ASSERT(tok.tokType() == Token::eBoolean);
    }

    void isStandardType() const {
        std::vector<std::string> standard_types;
        standard_types.emplace_back("bool");
        standard_types.emplace_back("char");
        standard_types.emplace_back("short");
        standard_types.emplace_back("int");
        standard_types.emplace_back("long");
        standard_types.emplace_back("float");
        standard_types.emplace_back("double");
        standard_types.emplace_back("size_t");

        std::vector<std::string>::const_iterator test_op, test_ops_end = standard_types.cend();
        for (test_op = standard_types.cbegin(); test_op != test_ops_end; ++test_op) {
            Token tok;
            tok.str(*test_op);
            ASSERT_EQUALS_MSG(true, tok.isStandardType(), "Failing standard type: " + *test_op);
        }

        // Negative test
        Token tok;
        tok.str("string");
        ASSERT_EQUALS(false, tok.isStandardType());

        // Change back to standard type
        tok.str("int");
        ASSERT_EQUALS(true, tok.isStandardType());

        // token can't be both type and variable
        tok.str("abc");
        tok.isStandardType(true);
        tok.varId(123);
        ASSERT_EQUALS(false, tok.isStandardType());
    }

    void updateProperties() const {
        Token tok;
        tok.str("foobar");

        ASSERT_EQUALS(true, tok.isName());
        ASSERT_EQUALS(false, tok.isNumber());

        tok.str("123456");

        ASSERT_EQUALS(false, tok.isName());
        ASSERT_EQUALS(true, tok.isNumber());
    }

    void isNameGuarantees1() const {
        Token tok;
        tok.str("Name");
        ASSERT_EQUALS(true, tok.isName());
    }

    void isNameGuarantees2() const {
        Token tok;
        tok.str("_name");
        ASSERT_EQUALS(true, tok.isName());
    }

    void isNameGuarantees3() const {
        Token tok;
        tok.str("_123");
        ASSERT_EQUALS(true, tok.isName());
    }

    void isNameGuarantees4() const {
        Token tok;
        tok.str("123456");
        ASSERT_EQUALS(false, tok.isName());
        ASSERT_EQUALS(true, tok.isNumber());
    }

    void isNameGuarantees5() const {
        Token tok;
        tok.str("a123456");
        ASSERT_EQUALS(true, tok.isName());
        ASSERT_EQUALS(false, tok.isNumber());
    }

    void isNameGuarantees6() const {
        Token tok;
        tok.str("$f");
        ASSERT_EQUALS(true, tok.isName());
    }

    void canFindMatchingBracketsNeedsOpen() const {
        givenACodeSampleToTokenize var("std::deque<std::set<int> > intsets;");

        const Token* const t = var.tokens()->findClosingBracket();
        ASSERT(t == nullptr);
    }

    void canFindMatchingBracketsInnerPair() const {
        givenACodeSampleToTokenize var("std::deque<std::set<int> > intsets;");

        const Token * const t = var.tokens()->tokAt(7)->findClosingBracket();
        ASSERT_EQUALS(">", t->str());
        ASSERT(var.tokens()->tokAt(9) == t);
    }

    void canFindMatchingBracketsOuterPair() const {
        givenACodeSampleToTokenize var("std::deque<std::set<int> > intsets;");

        const Token* const t = var.tokens()->tokAt(3)->findClosingBracket();
        ASSERT_EQUALS(">", t->str());
        ASSERT(var.tokens()->tokAt(10) == t);
    }

    void canFindMatchingBracketsWithTooManyClosing() const {
        givenACodeSampleToTokenize var("X< 1>2 > x1;");

        const Token* const t = var.tokens()->next()->findClosingBracket();
        ASSERT_EQUALS(">", t->str());
        ASSERT(var.tokens()->tokAt(3) == t);
    }

    void canFindMatchingBracketsWithTooManyOpening() const {
        givenACodeSampleToTokenize var("X < (2 < 1) > x1;");

        const Token* t = var.tokens()->next()->findClosingBracket();
        ASSERT(t != nullptr && t->str() == ">");

        t = var.tokens()->tokAt(4)->findClosingBracket();
        ASSERT(t == nullptr);
    }

    void findClosingBracket() const {
        givenACodeSampleToTokenize var("template<typename X, typename...Y> struct S : public Fred<Wilma<Y...>> {}");

        const Token* const t = var.tokens()->next()->findClosingBracket();
        ASSERT(Token::simpleMatch(t, "> struct"));
    }

    void expressionString() const {
        givenACodeSampleToTokenize var1("void f() { *((unsigned long long *)x) = 0; }");
        const Token *const tok1 = Token::findsimplematch(var1.tokens(), "*");
        ASSERT_EQUALS("*((unsigned long long*)x)", tok1->expressionString());

        givenACodeSampleToTokenize var2("typedef unsigned long long u64; void f() { *((u64 *)x) = 0; }");
        const Token *const tok2 = Token::findsimplematch(var2.tokens(), "*");
        ASSERT_EQUALS("*((unsigned long long*)x)", tok2->expressionString());

        givenACodeSampleToTokenize data3("void f() { return (t){1,2}; }");
        ASSERT_EQUALS("return(t){1,2}", data3.tokens()->tokAt(5)->expressionString());

        givenACodeSampleToTokenize data4("void f() { return L\"a\"; }");
        ASSERT_EQUALS("returnL\"a\"", data4.tokens()->tokAt(5)->expressionString());

        givenACodeSampleToTokenize data5("void f() { return U\"a\"; }");
        ASSERT_EQUALS("returnU\"a\"", data5.tokens()->tokAt(5)->expressionString());

        givenACodeSampleToTokenize data6("x = \"\\0\\x1\\x2\\x3\\x4\\x5\\x6\\x7\";");
        ASSERT_EQUALS("x=\"\\x00\\x01\\x02\\x03\\x04\\x05\\x06\\x07\"", data6.tokens()->next()->expressionString());
    }

    void hasKnownIntValue() const {
        // pointer might be NULL
        ValueFlow::Value v1(0);

        // pointer points at buffer that is 2 bytes
        ValueFlow::Value v2(2);
        v2.valueType = ValueFlow::Value::ValueType::BUFFER_SIZE;
        v2.setKnown();

        Token token;
        ASSERT_EQUALS(true, token.addValue(v1));
        ASSERT_EQUALS(true, token.addValue(v2));
        ASSERT_EQUALS(false, token.hasKnownIntValue());
    }
};

REGISTER_TEST(TestToken)
