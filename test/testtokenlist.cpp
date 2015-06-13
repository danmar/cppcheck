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
#include "settings.h"
#include "tokenlist.h"
#include "token.h"
#include <cstring>


class TestTokenList : public TestFixture {
public:
    TestTokenList() : TestFixture("TestTokenList") {
    }

private:

    void run() {

        TEST_CASE(testaddtoken);
    }

    // inspired by #5895
    void testaddtoken() {
        const std::string code = "0x89504e470d0a1a0a";
        Settings settings;
        TokenList tokenlist(&settings);
        tokenlist.addtoken(code, 1, 1, false);
        ASSERT_EQUALS("9894494448401390090", tokenlist.front()->str());
    }

};

REGISTER_TEST(TestTokenList)
