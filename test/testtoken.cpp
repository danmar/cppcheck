/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2024 Cppcheck team.
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

#include "fixture.h"
#include "helpers.h"
#include "standards.h"
#include "token.h"
#include "tokenlist.h"
#include "vfvalue.h"

#include <algorithm>
#include <string>
#include <vector>


class TestToken : public TestFixture {
public:
    TestToken() : TestFixture("TestToken") {
        list.setLang(Standards::Language::C);
    }

private:
    /*const*/ TokenList list{&settingsDefault};

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
        TEST_CASE(findClosingBracket2);
        TEST_CASE(findClosingBracket3);
        TEST_CASE(findClosingBracket4);

        TEST_CASE(expressionString);

        TEST_CASE(hasKnownIntValue);
    }

    void nextprevious() const {
        TokensFrontBack tokensFrontBack(list);
        auto *token = new Token(tokensFrontBack);
        token->str("1");
        (void)token->insertToken("2");
        (void)token->next()->insertToken("3");
        Token *last = token->tokAt(2);
        ASSERT_EQUALS(token->str(), "1");
        ASSERT_EQUALS(token->strAt(1), "2");
        // cppcheck-suppress redundantNextPrevious - this is intentional
        ASSERT_EQUALS(token->tokAt(2)->str(), "3");
        ASSERT_EQUALS_MSG(true, last->next() == nullptr, "Null was expected");

        ASSERT_EQUALS(last->str(), "3");
        ASSERT_EQUALS(last->strAt(-1), "2");
        // cppcheck-suppress redundantNextPrevious - this is intentional
        ASSERT_EQUALS(last->tokAt(-2)->str(), "1");
        ASSERT_EQUALS_MSG(true, token->previous() == nullptr, "Null was expected");

        TokenList::deleteTokens(token);
    }

#define MatchCheck(...) MatchCheck_(__FILE__, __LINE__, __VA_ARGS__)
    bool MatchCheck_(const char* file, int line, const std::string& code, const std::string& pattern, unsigned int varid = 0) {
        SimpleTokenizer tokenizer(settingsDefault, *this);
        const std::string code2 = ";" + code + ";";
        try {
            ASSERT_LOC(tokenizer.tokenize(code2), file, line);
        } catch (...) {}
        return Token::Match(tokenizer.tokens()->next(), pattern.c_str(), varid);
    }

    void multiCompare() const {
        // Test for found
        {
            TokensFrontBack tokensFrontBack(list);
            Token one(tokensFrontBack);
            one.str("one");
            ASSERT_EQUALS(1, Token::multiCompare(&one, "one|two", 0));
        }

        {
            TokensFrontBack tokensFrontBack(list);
            Token two(tokensFrontBack);
            two.str("two");
            ASSERT_EQUALS(1, Token::multiCompare(&two, "one|two", 0));
            ASSERT_EQUALS(1, Token::multiCompare(&two, "verybig|two|", 0));
        }

        // Test for empty string found
        {
            TokensFrontBack tokensFrontBack(list);
            Token notfound(tokensFrontBack);
            notfound.str("notfound");
            ASSERT_EQUALS(0, Token::multiCompare(&notfound, "one|two|", 0));

            // Test for not found
            ASSERT_EQUALS(-1, Token::multiCompare(&notfound, "one|two", 0));
        }

        {
            TokensFrontBack tokensFrontBack(list);
            Token s(tokensFrontBack);
            s.str("s");
            ASSERT_EQUALS(-1, Token::multiCompare(&s, "verybig|two", 0));
        }

        {
            TokensFrontBack tokensFrontBack(list);
            Token ne(tokensFrontBack);
            ne.str("ne");
            ASSERT_EQUALS(-1, Token::multiCompare(&ne, "one|two", 0));
        }

        {
            TokensFrontBack tokensFrontBack(list);
            Token a(tokensFrontBack);
            a.str("a");
            ASSERT_EQUALS(-1, Token::multiCompare(&a, "abc|def", 0));
        }

        {
            TokensFrontBack tokensFrontBack(list);
            Token abcd(tokensFrontBack);
            abcd.str("abcd");
            ASSERT_EQUALS(-1, Token::multiCompare(&abcd, "abc|def", 0));
        }

        {
            TokensFrontBack tokensFrontBack(list);
            Token def(tokensFrontBack);
            def.str("default");
            ASSERT_EQUALS(-1, Token::multiCompare(&def, "abc|def", 0));
        }

        // %op%
        {
            TokensFrontBack tokensFrontBack(list);
            Token plus(tokensFrontBack);
            plus.str("+");
            ASSERT_EQUALS(1, Token::multiCompare(&plus, "one|%op%", 0));
            ASSERT_EQUALS(1, Token::multiCompare(&plus, "%op%|two", 0));
        }
        {
            TokensFrontBack tokensFrontBack(list);
            Token x(tokensFrontBack);
            x.str("x");
            ASSERT_EQUALS(-1, Token::multiCompare(&x, "one|%op%", 0));
            ASSERT_EQUALS(-1, Token::multiCompare(&x, "%op%|two", 0));
        }

    }

    void multiCompare2() const { // #3294
        // Original pattern that failed: [[,(=<>+-*|&^] %num% [+-*/] %num% ]|,|)|;|=|%op%
        const SimpleTokenList toks("a == 1");
        ASSERT_EQUALS(true, Token::Match(toks.front(), "a =|%op%"));
    }

    void multiCompare3() const {
        // Original pattern that failed: "return|(|&&|%oror% %name% &&|%oror%|==|!=|<=|>=|<|>|-|%or% %name% )|&&|%oror%|;"
        // Code snippet that failed: "return lv@86 |= rv@87 ;"

        // Note: Also test "reverse" alternative pattern, two different code paths to handle it
        const SimpleTokenList toks("return a |= b ;");
        ASSERT_EQUALS(false, Token::Match(toks.front(), "return %name% xyz|%or% %name% ;"));
        ASSERT_EQUALS(false, Token::Match(toks.front(), "return %name% %or%|xyz %name% ;"));

        const SimpleTokenList toks2("return a | b ;");
        ASSERT_EQUALS(true, Token::Match(toks2.front(), "return %name% xyz|%or% %name% ;"));
        ASSERT_EQUALS(true, Token::Match(toks2.front(), "return %name% %or%|xyz %name% ;"));

        const SimpleTokenList toks3("return a || b ;");
        ASSERT_EQUALS(false, Token::Match(toks3.front(), "return %name% xyz|%or% %name% ;"));
        ASSERT_EQUALS(false, Token::Match(toks3.front(), "return %name% %or%|xyz %name% ;"));

        ASSERT_EQUALS(true, Token::Match(toks3.front(), "return %name% xyz|%oror% %name% ;"));
        ASSERT_EQUALS(true, Token::Match(toks3.front(), "return %name% %oror%|xyz %name% ;"));

        const SimpleTokenList toks4("a % b ;");
        ASSERT_EQUALS(true, Token::Match(toks4.front(), "%name% >>|<<|&|%or%|^|% %name% ;"));
        ASSERT_EQUALS(true, Token::Match(toks4.front(), "%name% %|>>|<<|&|%or%|^ %name% ;"));
        ASSERT_EQUALS(true, Token::Match(toks4.front(), "%name% >>|<<|&|%or%|%|^ %name% ;"));

        //%name%|%num% support
        const SimpleTokenList num("100");
        ASSERT_EQUALS(true, Token::Match(num.front(), "%num%|%name%"));
        ASSERT_EQUALS(true, Token::Match(num.front(), "%name%|%num%"));
        ASSERT_EQUALS(true, Token::Match(num.front(), "%name%|%num%|%bool%"));
        ASSERT_EQUALS(true, Token::Match(num.front(), "%name%|%bool%|%num%"));
        ASSERT_EQUALS(true, Token::Match(num.front(), "%name%|%bool%|%str%|%num%"));
        ASSERT_EQUALS(false, Token::Match(num.front(), "%bool%|%name%"));
        ASSERT_EQUALS(false, Token::Match(num.front(), "%type%|%bool%|%char%"));
        ASSERT_EQUALS(true, Token::Match(num.front(), "%type%|%bool%|100"));

        const SimpleTokenList numparen("( 100 )");
        ASSERT_EQUALS(true, Token::Match(numparen.front(), "(| %num%|%name% )|"));
        ASSERT_EQUALS(true, Token::Match(numparen.front(), "(| %name%|%num% )|"));
        ASSERT_EQUALS(true, Token::Match(numparen.front(), "(| %name%|%num%|%bool% )|"));
        ASSERT_EQUALS(true, Token::Match(numparen.front(), "(| %name%|%bool%|%num% )|"));
        ASSERT_EQUALS(true, Token::Match(numparen.front(), "(| %name%|%bool%|%str%|%num% )|"));
        ASSERT_EQUALS(false, Token::Match(numparen.front(), "(| %bool%|%name% )|"));

        ASSERT_EQUALS(true, Token::Match(numparen.front(), "(| 100 %num%|%name%| )|"));
        ASSERT_EQUALS(true, Token::Match(numparen.front(), "(| 100 %name%|%num%| )|"));
        ASSERT_EQUALS(true, Token::Match(numparen.front(), "(| 100 %name%|%num%|%bool%| )|"));
        ASSERT_EQUALS(true, Token::Match(numparen.front(), "(| 100 %name%|%bool%|%num%| )|"));
        ASSERT_EQUALS(true, Token::Match(numparen.front(), "(| 100 %name%|%bool%|%str%|%num%| )|"));
        ASSERT_EQUALS(true, Token::Match(numparen.front(), "(| 100 %bool%|%name%| )|"));
    }

    void multiCompare4() {
        const SimpleTokenizer var(*this, "std :: queue < int > foo ;");

        ASSERT_EQUALS(Token::eBracket, var.tokens()->tokAt(3)->tokType());
        ASSERT_EQUALS(Token::eBracket, var.tokens()->tokAt(5)->tokType());

        ASSERT_EQUALS(false, Token::Match(var.tokens(), "std :: queue %op%"));
        ASSERT_EQUALS(false, Token::Match(var.tokens(), "std :: queue x|%op%"));
        ASSERT_EQUALS(false, Token::Match(var.tokens(), "std :: queue %op%|x"));
    }

    void multiCompare5() const {
        TokensFrontBack tokensFrontBack(list);
        Token tok(tokensFrontBack);
        tok.str("||");
        ASSERT_EQUALS(true, Token::multiCompare(&tok, "+|%or%|%oror%", 0) >= 0);
    }

    void charTypes() const {
        TokensFrontBack tokensFrontBack(list);
        Token tok(tokensFrontBack);

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

        tok.str("'\\''");
        ASSERT_EQUALS(true, tok.isCChar());
        ASSERT_EQUALS(false, tok.isUtf8());
        ASSERT_EQUALS(false, tok.isUtf16());
        ASSERT_EQUALS(false, tok.isUtf32());
        ASSERT_EQUALS(false, tok.isLong());
        ASSERT_EQUALS(false, tok.isCMultiChar());

        tok.str("'\\r\\n'");
        ASSERT_EQUALS(false, tok.isCChar());
        ASSERT_EQUALS(false, tok.isUtf8());
        ASSERT_EQUALS(false, tok.isUtf16());
        ASSERT_EQUALS(false, tok.isUtf32());
        ASSERT_EQUALS(false, tok.isLong());
        ASSERT_EQUALS(true, tok.isCMultiChar());

        tok.str("'\\x10'");
        ASSERT_EQUALS(true, tok.isCChar());
        ASSERT_EQUALS(false, tok.isUtf8());
        ASSERT_EQUALS(false, tok.isUtf16());
        ASSERT_EQUALS(false, tok.isUtf32());
        ASSERT_EQUALS(false, tok.isLong());
        ASSERT_EQUALS(false, tok.isCMultiChar());
    }

    void stringTypes() const {
        TokensFrontBack tokensFrontBack(list);
        Token tok(tokensFrontBack);

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
        TokensFrontBack tokensFrontBack(list);
        Token tok(tokensFrontBack);

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
        TokensFrontBack tokensFrontBack(list);
        Token tok(tokensFrontBack);

        tok.str("\"\"");
        ASSERT_EQUALS(sizeof(""), Token::getStrSize(&tok, settingsDefault));

        tok.str("\"abc\"");
        ASSERT_EQUALS(sizeof("abc"), Token::getStrSize(&tok, settingsDefault));

        tok.str("\"\\0abc\"");
        ASSERT_EQUALS(sizeof("\0abc"), Token::getStrSize(&tok, settingsDefault));

        tok.str("\"\\\\\"");
        ASSERT_EQUALS(sizeof("\\"), Token::getStrSize(&tok, settingsDefault));
    }

    void strValue() const {
        TokensFrontBack tokensFrontBack(list);
        Token tok(tokensFrontBack);

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
        TokensFrontBack tokensFrontBack(list);
        Token tok(tokensFrontBack);

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
        TokensFrontBack listEnds(list);
        Token ** const tokensBack = &(listEnds.back);
        Token tok(listEnds);
        (void)tok.insertToken("aba");
        ASSERT_EQUALS(true, *tokensBack == tok.next());
        tok.deleteNext();
        ASSERT_EQUALS(true, *tokensBack == &tok);
    }

    void deleteFirst() const {
        TokensFrontBack listEnds(list);
        Token ** const tokensFront = &(listEnds.front);
        Token tok(listEnds);

        (void)tok.insertToken("aba");

        ASSERT_EQUALS(true, *tokensFront == tok.previous());
        tok.deletePrevious();
        ASSERT_EQUALS(true, *tokensFront == &tok);
    }

    void nextArgument() {
        const SimpleTokenizer example1(*this, "foo(1, 2, 3, 4);");
        ASSERT_EQUALS(true, Token::simpleMatch(example1.tokens()->tokAt(2)->nextArgument(), "2 , 3"));
        ASSERT_EQUALS(true, Token::simpleMatch(example1.tokens()->tokAt(4)->nextArgument(), "3 , 4"));

        const SimpleTokenizer example2(*this, "foo();");
        ASSERT_EQUALS(true, example2.tokens()->tokAt(2)->nextArgument() == nullptr);

        const SimpleTokenizer example3(*this, "foo(bar(a, b), 2, 3);");
        ASSERT_EQUALS(true, Token::simpleMatch(example3.tokens()->tokAt(2)->nextArgument(), "2 , 3"));

        const SimpleTokenizer example4(*this, "foo(x.i[1], \"\", 3);");
        ASSERT_EQUALS(true, Token::simpleMatch(example4.tokens()->tokAt(2)->nextArgument(), "\"\" , 3"));
    }

    void eraseTokens() const {
        SimpleTokenList code("begin ; { this code will be removed } end", Standards::Language::C);
        Token::eraseTokens(code.front()->next(), code.front()->tokAt(9));
        ASSERT_EQUALS("begin ; end", code.front()->stringifyList(nullptr, false));
    }


    void matchAny() const {
        const SimpleTokenList varBitOrVar("abc|def");
        ASSERT_EQUALS(true, Token::Match(varBitOrVar.front(), "%name% %or% %name%"));

        const SimpleTokenList varLogOrVar("abc||def");
        ASSERT_EQUALS(true, Token::Match(varLogOrVar.front(), "%name% %oror% %name%"));
    }

    void matchSingleChar() const {
        const SimpleTokenList singleChar("a");
        ASSERT_EQUALS(true, Token::Match(singleChar.front(), "[a|bc]"));
        ASSERT_EQUALS(false, Token::Match(singleChar.front(), "[d|ef]"));

        TokensFrontBack tokensFrontBack(list);
        Token multiChar(tokensFrontBack);
        multiChar.str("[ab");
        ASSERT_EQUALS(false, Token::Match(&multiChar, "[ab|def]"));
    }

    void matchNothingOrAnyNotElse() const {
        const SimpleTokenList empty_String("");
        ASSERT_EQUALS(true, Token::Match(empty_String.front(), "!!else"));
        ASSERT_EQUALS(false, Token::Match(empty_String.front(), "!!else something"));

        const SimpleTokenList ifSemicolon("if ;");
        ASSERT_EQUALS(true, Token::Match(ifSemicolon.front(), "if ; !!else"));

        const SimpleTokenList ifSemicolonSomething("if ; something");
        ASSERT_EQUALS(true, Token::Match(ifSemicolonSomething.front(), "if ; !!else"));

        const SimpleTokenList justElse("else");
        ASSERT_EQUALS(false, Token::Match(justElse.front(), "!!else"));

        const SimpleTokenList ifSemicolonElse("if ; else");
        ASSERT_EQUALS(false, Token::Match(ifSemicolonElse.front(), "if ; !!else"));
    }

    void matchType() {
        const SimpleTokenList type("abc");
        ASSERT_EQUALS(true, Token::Match(type.front(), "%type%"));

        const SimpleTokenizer isVar(*this, "int a = 3 ;");
        ASSERT_EQUALS(true, Token::Match(isVar.tokens(), "%type%"));
        ASSERT_EQUALS(true, Token::Match(isVar.tokens(), "%type% %name%"));
        ASSERT_EQUALS(false, Token::Match(isVar.tokens(), "%type% %type%"));

        // TODO: %type% should not match keywords other than fundamental types
        const SimpleTokenList noType1_cpp("delete");
        ASSERT_EQUALS(true, Token::Match(noType1_cpp.front(), "%type%"));

        const SimpleTokenList noType1_c("delete", Standards::Language::C);
        ASSERT_EQUALS(true, Token::Match(noType1_c.front(), "%type%"));

        const SimpleTokenList noType2("void delete");
        ASSERT_EQUALS(true, Token::Match(noType2.front(), "!!foo %type%"));
    }

    void matchChar() const {
        const SimpleTokenList chr1("'a'");
        ASSERT_EQUALS(true, Token::Match(chr1.front(), "%char%"));

        const SimpleTokenList chr2("'1'");
        ASSERT_EQUALS(true, Token::Match(chr2.front(), "%char%"));

        const SimpleTokenList noChr("\"10\"");
        ASSERT_EQUALS(false, Token::Match(noChr.front(), "%char%"));
    }

    void matchCompOp() const {
        const SimpleTokenList comp1("<=");
        ASSERT_EQUALS(true, Token::Match(comp1.front(), "%comp%"));

        const SimpleTokenList comp2(">");
        ASSERT_EQUALS(true, Token::Match(comp2.front(), "%comp%"));

        const SimpleTokenList noComp("=");
        ASSERT_EQUALS(false, Token::Match(noComp.front(), "%comp%"));
    }

    void matchStr() const {
        const SimpleTokenList noStr1("abc");
        ASSERT_EQUALS(false, Token::Match(noStr1.front(), "%str%"));

        const SimpleTokenList noStr2("'a'");
        ASSERT_EQUALS(false, Token::Match(noStr2.front(), "%str%"));

        const SimpleTokenList str("\"abc\"");
        ASSERT_EQUALS(true, Token::Match(str.front(), "%str%"));

        // Empty string
        const SimpleTokenList emptyStr("\"\"");
        ASSERT_EQUALS(true, Token::Match(emptyStr.front(), "%str%"));
    }

    void matchVarid() {
        const SimpleTokenizer var(*this, "int a ; int b ;");

        // Varid == 0 should throw exception
        ASSERT_THROW_INTERNAL_EQUALS((void)Token::Match(var.tokens(), "%type% %varid% ; %type% %name%", 0),INTERNAL,"Internal error. Token::Match called with varid 0. Please report this to Cppcheck developers");

        ASSERT_EQUALS(true, Token::Match(var.tokens(), "%type% %varid% ; %type% %name%", 1));
        ASSERT_EQUALS(true, Token::Match(var.tokens(), "%type% %name% ; %type% %varid%", 2));

        // Try to match two different varids in one match call
        ASSERT_EQUALS(false, Token::Match(var.tokens(), "%type% %varid% ; %type% %varid%", 2));

        // %var% matches with every varid other than 0
        ASSERT_EQUALS(true, Token::Match(var.tokens(), "%type% %var% ;"));
        ASSERT_EQUALS(false, Token::Match(var.tokens(), "%var% %var% ;"));
    }

    void matchNumeric() const {
        const SimpleTokenList nonNumeric("abc");
        ASSERT_EQUALS(false, Token::Match(nonNumeric.front(), "%num%"));

        const SimpleTokenList msLiteral("5ms"); // #11438
        ASSERT_EQUALS(false, Token::Match(msLiteral.front(), "%num%"));

        const SimpleTokenList sLiteral("3s");
        ASSERT_EQUALS(false, Token::Match(sLiteral.front(), "%num%"));

        const SimpleTokenList octal("0123");
        ASSERT_EQUALS(true, Token::Match(octal.front(), "%num%"));

        const SimpleTokenList decimal("4567");
        ASSERT_EQUALS(true, Token::Match(decimal.front(), "%num%"));

        const SimpleTokenList hexadecimal("0xDEADBEEF");
        ASSERT_EQUALS(true, Token::Match(hexadecimal.front(), "%num%"));

        const SimpleTokenList floatingPoint("0.0f");
        ASSERT_EQUALS(true, Token::Match(floatingPoint.front(), "%num%"));

        const SimpleTokenList signedLong("0L");
        ASSERT_EQUALS(true, Token::Match(signedLong.front(), "%num%"));

        const SimpleTokenList negativeSignedLong("-0L");
        ASSERT_EQUALS(true, Token::Match(negativeSignedLong.front(), "- %num%"));

        const SimpleTokenList positiveSignedLong("+0L");
        ASSERT_EQUALS(true, Token::Match(positiveSignedLong.front(), "+ %num%"));

        const SimpleTokenList unsignedInt("0U");
        ASSERT_EQUALS(true, Token::Match(unsignedInt.front(), "%num%"));

        const SimpleTokenList unsignedLong("0UL");
        ASSERT_EQUALS(true, Token::Match(unsignedLong.front(), "%num%"));

        const SimpleTokenList unsignedLongLong("0ULL");
        ASSERT_EQUALS(true, Token::Match(unsignedLongLong.front(), "%num%"));

        const SimpleTokenList positive("+666");
        ASSERT_EQUALS(true, Token::Match(positive.front(), "+ %num%"));

        const SimpleTokenList negative("-42");
        ASSERT_EQUALS(true, Token::Match(negative.front(), "- %num%"));

        const SimpleTokenList negativeNull("-.0");
        ASSERT_EQUALS(true, Token::Match(negativeNull.front(), "- %num%"));

        const SimpleTokenList positiveNull("+.0");
        ASSERT_EQUALS(true, Token::Match(positiveNull.front(), "+ %num%"));
    }


    void matchBoolean() const {
        const SimpleTokenList yes("YES");
        ASSERT_EQUALS(false, Token::Match(yes.front(), "%bool%"));

        const SimpleTokenList positive("true");
        ASSERT_EQUALS(true, Token::Match(positive.front(), "%bool%"));

        const SimpleTokenList negative("false");
        ASSERT_EQUALS(true, Token::Match(negative.front(), "%bool%"));
    }

    void matchOr() const {
        const SimpleTokenList bitwiseOr(";|;");
        // cppcheck-suppress simplePatternError - this is intentional
        ASSERT_EQUALS(true,  Token::Match(bitwiseOr.front(), "; %or%"));
        ASSERT_EQUALS(true,  Token::Match(bitwiseOr.front(), "; %op%"));
        // cppcheck-suppress simplePatternError - this is intentional
        ASSERT_EQUALS(false, Token::Match(bitwiseOr.front(), "; %oror%"));

        const SimpleTokenList bitwiseOrAssignment(";|=;");
        // cppcheck-suppress simplePatternError - this is intentional
        ASSERT_EQUALS(false,  Token::Match(bitwiseOrAssignment.front(), "; %or%"));
        ASSERT_EQUALS(true,  Token::Match(bitwiseOrAssignment.front(), "; %op%"));
        // cppcheck-suppress simplePatternError - this is intentional
        ASSERT_EQUALS(false, Token::Match(bitwiseOrAssignment.front(), "; %oror%"));

        const SimpleTokenList logicalOr(";||;");
        // cppcheck-suppress simplePatternError - this is intentional
        ASSERT_EQUALS(false, Token::Match(logicalOr.front(), "; %or%"));
        ASSERT_EQUALS(true,  Token::Match(logicalOr.front(), "; %op%"));
        // cppcheck-suppress simplePatternError - this is intentional
        ASSERT_EQUALS(true,  Token::Match(logicalOr.front(), "; %oror%"));
        ASSERT_EQUALS(true,  Token::Match(logicalOr.front(), "; &&|%oror%"));
        ASSERT_EQUALS(true,  Token::Match(logicalOr.front(), "; %oror%|&&"));

        const SimpleTokenList logicalAnd(";&&;");
        ASSERT_EQUALS(true, Token::simpleMatch(logicalAnd.front(), "; &&"));
        ASSERT_EQUALS(true, Token::Match(logicalAnd.front(), "; &&|%oror%"));
        ASSERT_EQUALS(true, Token::Match(logicalAnd.front(), "; %oror%|&&"));
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

        for (auto other_op = other_ops.cbegin(); other_op != other_ops.cend(); ++other_op) {
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

        for (auto other_op = other_ops.cbegin(); other_op != other_ops.cend(); ++other_op) {
            ASSERT_EQUALS_MSG(false, MatchCheck(*other_op, "%cop%"), "Failing other operator: " + *other_op);
        }
    }


    void isArithmeticalOp() const {
        for (auto test_op = arithmeticalOps.cbegin(); test_op != arithmeticalOps.cend(); ++test_op) {
            TokensFrontBack tokensFrontBack(list);
            Token tok(tokensFrontBack);
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

        for (auto other_op = other_ops.cbegin(); other_op != other_ops.cend(); ++other_op) {
            TokensFrontBack tokensFrontBack(list);
            Token tok(tokensFrontBack);
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

        for (auto test_op = test_ops.cbegin(); test_op != test_ops.cend(); ++test_op) {
            TokensFrontBack tokensFrontBack(list);
            Token tok(tokensFrontBack);
            tok.str(*test_op);
            ASSERT_EQUALS(true, tok.isOp());
        }

        // Negative test against other operators
        std::vector<std::string> other_ops;
        append_vector(other_ops, extendedOps);

        for (auto other_op = other_ops.cbegin(); other_op != other_ops.cend(); ++other_op) {
            TokensFrontBack tokensFrontBack(list);
            Token tok(tokensFrontBack);
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

        for (auto test_op = test_ops.cbegin(); test_op != test_ops.cend(); ++test_op) {
            TokensFrontBack tokensFrontBack(list);
            Token tok(tokensFrontBack);
            tok.str(*test_op);
            ASSERT_EQUALS(true, tok.isConstOp());
        }

        // Negative test against other operators
        std::vector<std::string> other_ops;
        append_vector(other_ops, extendedOps);
        append_vector(other_ops, assignmentOps);

        for (auto other_op = other_ops.cbegin(); other_op != other_ops.cend(); ++other_op) {
            TokensFrontBack tokensFrontBack(list);
            Token tok(tokensFrontBack);
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

        for (auto test_op = test_ops.cbegin(); test_op != test_ops.cend(); ++test_op) {
            TokensFrontBack tokensFrontBack(list);
            Token tok(tokensFrontBack);
            tok.str(*test_op);
            ASSERT_EQUALS(true, tok.isExtendedOp());
        }

        // Negative test against assignment operators
        for (auto other_op = assignmentOps.cbegin(); other_op != assignmentOps.cend(); ++other_op) {
            TokensFrontBack tokensFrontBack(list);
            Token tok(tokensFrontBack);
            tok.str(*other_op);
            ASSERT_EQUALS_MSG(false, tok.isExtendedOp(), "Failing assignment operator: " + *other_op);
        }
    }

    void isAssignmentOp() const {
        for (auto test_op = assignmentOps.cbegin(); test_op != assignmentOps.cend(); ++test_op) {
            TokensFrontBack tokensFrontBack(list);
            Token tok(tokensFrontBack);
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

        for (auto other_op = other_ops.cbegin(); other_op != other_ops.cend(); ++other_op) {
            TokensFrontBack tokensFrontBack(list);
            Token tok(tokensFrontBack);
            tok.str(*other_op);
            ASSERT_EQUALS_MSG(false, tok.isAssignmentOp(), "Failing assignment operator: " + *other_op);
        }
    }

    void operators() const {
        for (auto test_op = extendedOps.cbegin(); test_op != extendedOps.cend(); ++test_op) {
            TokensFrontBack tokensFrontBack(list);
            Token tok(tokensFrontBack);
            tok.str(*test_op);
            ASSERT_EQUALS(Token::eExtendedOp, tok.tokType());
        }
        for (auto test_op = logicalOps.cbegin(); test_op != logicalOps.cend(); ++test_op) {
            TokensFrontBack tokensFrontBack(list);
            Token tok(tokensFrontBack);
            tok.str(*test_op);
            ASSERT_EQUALS(Token::eLogicalOp, tok.tokType());
        }
        for (auto test_op = bitOps.cbegin(); test_op != bitOps.cend(); ++test_op) {
            TokensFrontBack tokensFrontBack(list);
            Token tok(tokensFrontBack);
            tok.str(*test_op);
            ASSERT_EQUALS(Token::eBitOp, tok.tokType());
        }
        for (auto test_op = comparisonOps.cbegin(); test_op != comparisonOps.cend(); ++test_op) {
            TokensFrontBack tokensFrontBack(list);
            Token tok(tokensFrontBack);
            tok.str(*test_op);
            ASSERT_EQUALS(Token::eComparisonOp, tok.tokType());
        }
        TokensFrontBack tokensFrontBack(list);
        Token tok(tokensFrontBack);
        tok.str("++");
        ASSERT_EQUALS(Token::eIncDecOp, tok.tokType());
        tok.str("--");
        ASSERT_EQUALS(Token::eIncDecOp, tok.tokType());
    }

    void literals() const {
        TokensFrontBack tokensFrontBack(list);
        Token tok(tokensFrontBack);

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

        for (auto test_op = standard_types.cbegin(); test_op != standard_types.cend(); ++test_op) {
            TokensFrontBack tokensFrontBack(list);
            Token tok(tokensFrontBack);
            tok.str(*test_op);
            ASSERT_EQUALS_MSG(true, tok.isStandardType(), "Failing standard type: " + *test_op);
        }

        // Negative test
        TokensFrontBack tokensFrontBack(list);
        Token tok(tokensFrontBack);
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
        TokensFrontBack tokensFrontBack(list);
        Token tok(tokensFrontBack);
        tok.str("foobar");

        ASSERT_EQUALS(true, tok.isName());
        ASSERT_EQUALS(false, tok.isNumber());

        tok.str("123456");

        ASSERT_EQUALS(false, tok.isName());
        ASSERT_EQUALS(true, tok.isNumber());
    }

    void isNameGuarantees1() const {
        TokensFrontBack tokensFrontBack(list);
        Token tok(tokensFrontBack);
        tok.str("Name");
        ASSERT_EQUALS(true, tok.isName());
    }

    void isNameGuarantees2() const {
        TokensFrontBack tokensFrontBack(list);
        Token tok(tokensFrontBack);
        tok.str("_name");
        ASSERT_EQUALS(true, tok.isName());
    }

    void isNameGuarantees3() const {
        TokensFrontBack tokensFrontBack(list);
        Token tok(tokensFrontBack);
        tok.str("_123");
        ASSERT_EQUALS(true, tok.isName());
    }

    void isNameGuarantees4() const {
        TokensFrontBack tokensFrontBack(list);
        Token tok(tokensFrontBack);
        tok.str("123456");
        ASSERT_EQUALS(false, tok.isName());
        ASSERT_EQUALS(true, tok.isNumber());
    }

    void isNameGuarantees5() const {
        TokensFrontBack tokensFrontBack(list);
        Token tok(tokensFrontBack);
        tok.str("a123456");
        ASSERT_EQUALS(true, tok.isName());
        ASSERT_EQUALS(false, tok.isNumber());
    }

    void isNameGuarantees6() const {
        TokensFrontBack tokensFrontBack(list);
        Token tok(tokensFrontBack);
        tok.str("$f");
        ASSERT_EQUALS(true, tok.isName());
    }

    void canFindMatchingBracketsNeedsOpen() {
        const SimpleTokenizer var(*this, "std::deque<std::set<int> > intsets;");

        const Token* const t = var.tokens()->findClosingBracket();
        ASSERT(t == nullptr);
    }

    void canFindMatchingBracketsInnerPair() {
        const SimpleTokenizer var(*this, "std::deque<std::set<int> > intsets;");

        const Token * const t = var.tokens()->tokAt(7)->findClosingBracket();
        ASSERT_EQUALS(">", t->str());
        ASSERT(var.tokens()->tokAt(9) == t);
    }

    void canFindMatchingBracketsOuterPair() {
        const SimpleTokenizer var(*this, "std::deque<std::set<int> > intsets;");

        const Token* const t = var.tokens()->tokAt(3)->findClosingBracket();
        ASSERT_EQUALS(">", t->str());
        ASSERT(var.tokens()->tokAt(10) == t);
    }

    void canFindMatchingBracketsWithTooManyClosing() {
        const SimpleTokenizer var(*this, "X< 1>2 > x1;");

        const Token* const t = var.tokens()->next()->findClosingBracket();
        ASSERT_EQUALS(">", t->str());
        ASSERT(var.tokens()->tokAt(3) == t);
    }

    void canFindMatchingBracketsWithTooManyOpening() {
        const SimpleTokenizer var(*this, "X < (2 < 1) > x1;");

        const Token* t = var.tokens()->next()->findClosingBracket();
        ASSERT(t != nullptr && t->str() == ">");

        t = var.tokens()->tokAt(4)->findClosingBracket();
        ASSERT(t == nullptr);
    }

    void findClosingBracket() {
        const SimpleTokenizer var(*this, "template<typename X, typename...Y> struct S : public Fred<Wilma<Y...>> {}");

        const Token* const t = var.tokens()->next()->findClosingBracket();
        ASSERT(Token::simpleMatch(t, "> struct"));
    }

    void findClosingBracket2() {
        const SimpleTokenizer var(*this, "const auto g = []<typename T>() {};\n"); // #11275

        const Token* const t = Token::findsimplematch(var.tokens(), "<");
        ASSERT(t && Token::simpleMatch(t->findClosingBracket(), ">"));
    }

    void findClosingBracket3() {
        const SimpleTokenizer var(*this, // #12789
                                  "template <size_t I = 0, typename... ArgsT, std::enable_if_t<I < sizeof...(ArgsT)>* = nullptr>\n"
                                  "void f();\n");
        const Token* const t = Token::findsimplematch(var.tokens(), "<");
        ASSERT(t && Token::simpleMatch(t->findClosingBracket(), ">"));
    }

    void findClosingBracket4() {
        const SimpleTokenizer var(*this, // #12923
                                  "template<template<class E> class T = std::vector, class U = std::vector<int>, class V = void>\n"
                                  "class C;\n");
        const Token *const t = Token::findsimplematch(var.tokens(), "<");
        ASSERT(t);
        const Token *const closing = t->findClosingBracket();
        ASSERT(closing && closing == var.tokens()->tokAt(28));
    }

    void expressionString() {
        const SimpleTokenizer var1(*this, "void f() { *((unsigned long long *)x) = 0; }");
        const Token *const tok1 = Token::findsimplematch(var1.tokens(), "*");
        ASSERT_EQUALS("*((unsigned long long*)x)", tok1->expressionString());

        const SimpleTokenizer var2(*this, "typedef unsigned long long u64; void f() { *((u64 *)x) = 0; }");
        const Token *const tok2 = Token::findsimplematch(var2.tokens(), "*");
        ASSERT_EQUALS("*((unsigned long long*)x)", tok2->expressionString());

        const SimpleTokenizer data3(*this, "void f() { return (t){1,2}; }");
        ASSERT_EQUALS("return(t){1,2}", data3.tokens()->tokAt(5)->expressionString());

        const SimpleTokenizer data4(*this, "void f() { return L\"a\"; }");
        ASSERT_EQUALS("returnL\"a\"", data4.tokens()->tokAt(5)->expressionString());

        const SimpleTokenizer data5(*this, "void f() { return U\"a\"; }");
        ASSERT_EQUALS("returnU\"a\"", data5.tokens()->tokAt(5)->expressionString());

        const SimpleTokenizer data6(*this, "x = \"\\0\\x1\\x2\\x3\\x4\\x5\\x6\\x7\";");
        ASSERT_EQUALS("x=\"\\x00\\x01\\x02\\x03\\x04\\x05\\x06\\x07\"", data6.tokens()->next()->expressionString());
    }

    void hasKnownIntValue() const {
        // pointer might be NULL
        ValueFlow::Value v1(0);

        // pointer points at buffer that is 2 bytes
        ValueFlow::Value v2(2);
        v2.valueType = ValueFlow::Value::ValueType::BUFFER_SIZE;
        v2.setKnown();

        TokensFrontBack tokensFrontBack(list);
        Token token(tokensFrontBack);
        ASSERT_EQUALS(true, token.addValue(v1));
        ASSERT_EQUALS(true, token.addValue(v2));
        ASSERT_EQUALS(false, token.hasKnownIntValue());
    }
};

REGISTER_TEST(TestToken)
