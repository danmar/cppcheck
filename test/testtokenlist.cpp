/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2018 Cppcheck team.
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
#include "token.h"
#include "tokenlist.h"

#include <sstream>
#include <string>

class TestTokenList : public TestFixture {
public:
    TestTokenList() : TestFixture("TestTokenList") {
    }

private:
    Settings settings;

    void run() override {
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
