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




class TestTokenIterators : public TestFixture {
public:
    TestTokenIterators() : TestFixture("TestTokenIterators") {}

private:

    void run() override {
    }

    std::shared_ptr<Tokenizer> tokenize(const char code[]) {
        Settings settings0;
        std::shared_ptr<Tokenizer> tokenizer = std::make_shared<Tokenizer>(&settings0, this);
        std::istringstream istr(code);
        tokenizer->tokenize(istr, "test.cpp");

        return tokenizer;
    }

};

REGISTER_TEST(TestTokenIterators)
