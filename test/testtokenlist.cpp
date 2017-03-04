/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2016 Cppcheck team.
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
#include "path.h"
#include "settings.h"
#include "tokenlist.h"
#include "token.h"
#include <cstring>
#include <sstream>


class TestTokenList : public TestFixture {
public:
    TestTokenList() : TestFixture("TestTokenList") {
    }

private:
    Settings settings;

    void run() {
        TEST_CASE(line1); // Ticket #4408
        TEST_CASE(line2); // Ticket #5423
        TEST_CASE(testaddtoken1);
        TEST_CASE(testaddtoken2);
        TEST_CASE(inc);
    }

    // inspired by #5895
    void testaddtoken1() {
        const std::string code = "0x89504e470d0a1a0a";
        TokenList tokenlist(&settings);
        tokenlist.addtoken(code, 1, 1, false);
        ASSERT_EQUALS("9894494448401390090U", tokenlist.front()->str());
        // that is supposed to break on 32bit
        //unsigned long numberUL(0);
        //std::istringstream(tokenlist.front()->str()) >> numberUL;
        //ASSERT_EQUALS(9894494448401390090U, numberUL);
        unsigned long long numberULL(0);
        std::istringstream(tokenlist.front()->str()) >> numberULL;
        ASSERT_EQUALS(9894494448401390090U, numberULL);
    }

    void testaddtoken2() {
        const std::string code = "0xF0000000";
        settings.int_bit = 32;
        TokenList tokenlist(&settings);
        tokenlist.addtoken(code, 1, 1, false);
        ASSERT_EQUALS("4026531840U", tokenlist.front()->str());
    }

    void line1() const {
        // Test for Ticket #4408
        const char code[] = "#file \"c:\\a.h\"\n"
                            "first\n"
                            "#line 5\n"
                            "second\n"
                            "#line not-a-number\n"
                            "third\n"
                            "#line 100 \"i.h\"\n"
                            "fourth\n"
                            "fifth\n"
                            "#endfile\n";

        errout.str("");

        TokenList tokenList(&settings);
        std::istringstream istr(code);
        bool res = tokenList.createTokens(istr, "a.cpp");
        ASSERT_EQUALS(res, true);

        for (const Token *tok = tokenList.front(); tok; tok = tok->next()) {
            if (tok->str() == "first")
                ASSERT_EQUALS(1, tok->linenr());
            if (tok->str() == "second")
                ASSERT_EQUALS(5, tok->linenr());
            if (tok->str() == "third")
                ASSERT_EQUALS(7, tok->linenr());
            if (tok->str() == "fourth")
                ASSERT_EQUALS(100, tok->linenr());
            if (tok->str() == "fifth")
                ASSERT_EQUALS(101, tok->linenr());
        }
    }

    void line2() const {
        const char code[] = "#line 8 \"c:\\a.h\"\n"
                            "123\n";

        errout.str("");

        // tokenize..
        TokenList tokenlist(&settings);
        std::istringstream istr(code);
        tokenlist.createTokens(istr, "a.cpp");

        ASSERT_EQUALS(Path::toNativeSeparators("[c:\\a.h:8]"), tokenlist.fileLine(tokenlist.front()));
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
};

REGISTER_TEST(TestTokenList)
