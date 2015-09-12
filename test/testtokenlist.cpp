/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2015 Daniel Marjamäki and Cppcheck team.
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

    void run() {
        TEST_CASE(line1); // Ticket #4408
        TEST_CASE(line2); // Ticket #5423
        TEST_CASE(testaddtoken);
    }

    // inspired by #5895
    void testaddtoken() {
        const std::string code = "0x89504e470d0a1a0a";
        Settings settings;
        TokenList tokenlist(&settings);
        tokenlist.addtoken(code, 1, 1, false);
        ASSERT_EQUALS("9894494448401390090", tokenlist.front()->str());
        // that is supposed to break on 32bit
        //unsigned long numberUL(0);
        //std::istringstream(tokenlist.front()->str()) >> numberUL;
        //ASSERT_EQUALS(9894494448401390090U, numberUL);
        unsigned long long numberULL(0);
        std::istringstream(tokenlist.front()->str()) >> numberULL;
        ASSERT_EQUALS(9894494448401390090U, numberULL);
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

        Settings settings;

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

        const Settings settings;

        // tokenize..
        TokenList tokenlist(&settings);
        std::istringstream istr(code);
        tokenlist.createTokens(istr, "a.cpp");

        ASSERT_EQUALS(Path::toNativeSeparators("[c:\\a.h:8]"), tokenlist.fileLine(tokenlist.front()));
    }



};

REGISTER_TEST(TestTokenList)
