/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2022 Cppcheck team.
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
#include "tokeniterators.h"
#include "settings.h"
#include "tokenize.h"


namespace {

    const char CODE_1[] =
        "void f {"
        "  skipThis(arg1, arg2, fail, arg4, arg5);"
        "  reachThis();"
        "}"
    ;
}


class TestTokenIterators : public TestFixture {
public:
    TestTokenIterators() : TestFixture("TestTokenIterators") {}

private:

    void run() override {
        //TEST_CASE(checkIteratorJumpInsideLoop);
        //TEST_CASE(checkAssignAnotherIterator);
    }

    std::shared_ptr<Tokenizer> tokenize(const char code[]) {
        Settings settings0;
        std::shared_ptr<Tokenizer> tokenizer = std::make_shared<Tokenizer>(&settings0, this);
        std::istringstream istr(code);
        tokenizer->tokenize(istr, "test.cpp");

        return tokenizer;
    }

    /*void checkIteratorJumpInsideLoop() {
        auto tokenizer = tokenize(CODE_1);

        bool expectedTokenReached = false;
        for (auto tok : IterateTokens(tokenizer->list)) {
            ASSERT(tok->str() != "fail");
            if (tok->str() == "reachThis") {
                expectedTokenReached = true;
            }
            if (Token::simpleMatch(tok, "skipThis (")) {
                const Token* skipTo = tok->next()->link();
                tok = skipTo;
                continue;
            }
        }
        ASSERT(expectedTokenReached);
    }

    void checkAssignAnotherIterator() {
        auto tokenizer = tokenize(CODE_1);

        for (auto tok : IterateTokens(tokenizer->list)) {
            if (tok->str() == "skipThis") {
                for (auto tok2 : IterateTokens(tokenizer->list)) {
                    if (tok2->str() == "reachThis") {
                        tok = tok2;
                        ++tok2;
                        ASSERT(tok->str() != tok2->str());
                    }
                }
                // tok position should not be tangled with tok2
                ASSERT(tok->str() == "reachThis");
                ++tok;
                ASSERT(tok->previous()->str() == "reachThis");
            }
        }
    }*/
};

REGISTER_TEST(TestTokenIterators)
