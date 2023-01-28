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

#include "helpers.h"
#include "fixture.h"
#include "utils.h"

const Settings givenACodeSampleToTokenize::settings;

class TestUtils : public TestFixture {
public:
    TestUtils() : TestFixture("TestUtils") {}

private:
    void run() override {
        TEST_CASE(isValidGlobPattern);
        TEST_CASE(matchglob);
        TEST_CASE(isStringLiteral);
        TEST_CASE(isCharLiteral);
    }

    void isValidGlobPattern() const {
        ASSERT_EQUALS(true, ::isValidGlobPattern("*"));
        ASSERT_EQUALS(true, ::isValidGlobPattern("*x"));
        ASSERT_EQUALS(true, ::isValidGlobPattern("x*"));
        ASSERT_EQUALS(true, ::isValidGlobPattern("*/x/*"));
        ASSERT_EQUALS(true, ::isValidGlobPattern("x/*/z"));
        ASSERT_EQUALS(false, ::isValidGlobPattern("**"));
        ASSERT_EQUALS(false, ::isValidGlobPattern("**x"));
        ASSERT_EQUALS(false, ::isValidGlobPattern("x**"));

        ASSERT_EQUALS(true, ::isValidGlobPattern("?"));
        ASSERT_EQUALS(true, ::isValidGlobPattern("?x"));
        ASSERT_EQUALS(true, ::isValidGlobPattern("x?"));
        ASSERT_EQUALS(true, ::isValidGlobPattern("?/x/?"));
        ASSERT_EQUALS(true, ::isValidGlobPattern("x/?/z"));
        ASSERT_EQUALS(false, ::isValidGlobPattern("??"));
        ASSERT_EQUALS(false, ::isValidGlobPattern("??x"));
        ASSERT_EQUALS(false, ::isValidGlobPattern("x??"));
    }

    void matchglob() const {
        ASSERT_EQUALS(true, ::matchglob("*", "xyz"));
        ASSERT_EQUALS(true, ::matchglob("x*", "xyz"));
        ASSERT_EQUALS(true, ::matchglob("*z", "xyz"));
        ASSERT_EQUALS(true, ::matchglob("*y*", "xyz"));
        ASSERT_EQUALS(true, ::matchglob("*y*", "yz"));
        ASSERT_EQUALS(false, ::matchglob("*y*", "abc"));
        ASSERT_EQUALS(true, ::matchglob("*", "x/y/z"));
        ASSERT_EQUALS(true, ::matchglob("*/y/z", "x/y/z"));

        ASSERT_EQUALS(false, ::matchglob("?", "xyz"));
        ASSERT_EQUALS(false, ::matchglob("x?", "xyz"));
        ASSERT_EQUALS(false, ::matchglob("?z", "xyz"));
        ASSERT_EQUALS(true, ::matchglob("?y?", "xyz"));
        ASSERT_EQUALS(true, ::matchglob("?/?/?", "x/y/z"));
    }

    void isStringLiteral() const {
        // empty
        ASSERT_EQUALS(false, ::isStringLiteral(""));

        // no literals
        ASSERT_EQUALS(false, ::isStringLiteral("u8"));
        ASSERT_EQUALS(false, ::isStringLiteral("u"));
        ASSERT_EQUALS(false, ::isStringLiteral("U"));
        ASSERT_EQUALS(false, ::isStringLiteral("L"));

        // incomplete string literals
        ASSERT_EQUALS(false, ::isStringLiteral("\""));
        ASSERT_EQUALS(false, ::isStringLiteral("u8\""));
        ASSERT_EQUALS(false, ::isStringLiteral("u\""));
        ASSERT_EQUALS(false, ::isStringLiteral("U\""));
        ASSERT_EQUALS(false, ::isStringLiteral("L\""));

        // valid string literals
        ASSERT_EQUALS(true, ::isStringLiteral("\"\""));
        ASSERT_EQUALS(true, ::isStringLiteral("u8\"\""));
        ASSERT_EQUALS(true, ::isStringLiteral("u\"\""));
        ASSERT_EQUALS(true, ::isStringLiteral("U\"\""));
        ASSERT_EQUALS(true, ::isStringLiteral("L\"\""));
        ASSERT_EQUALS(true, ::isStringLiteral("\"t\""));
        ASSERT_EQUALS(true, ::isStringLiteral("u8\"t\""));
        ASSERT_EQUALS(true, ::isStringLiteral("u\"t\""));
        ASSERT_EQUALS(true, ::isStringLiteral("U\"t\""));
        ASSERT_EQUALS(true, ::isStringLiteral("L\"t\""));
        ASSERT_EQUALS(true, ::isStringLiteral("\"test\""));
        ASSERT_EQUALS(true, ::isStringLiteral("u8\"test\""));
        ASSERT_EQUALS(true, ::isStringLiteral("u\"test\""));
        ASSERT_EQUALS(true, ::isStringLiteral("U\"test\""));
        ASSERT_EQUALS(true, ::isStringLiteral("L\"test\""));

        // incomplete char literals
        ASSERT_EQUALS(false, ::isStringLiteral("'"));
        ASSERT_EQUALS(false, ::isStringLiteral("u8'"));
        ASSERT_EQUALS(false, ::isStringLiteral("u'"));
        ASSERT_EQUALS(false, ::isStringLiteral("U'"));
        ASSERT_EQUALS(false, ::isStringLiteral("L'"));

        // valid char literals
        ASSERT_EQUALS(false, ::isStringLiteral("'t'"));
        ASSERT_EQUALS(false, ::isStringLiteral("u8't'"));
        ASSERT_EQUALS(false, ::isStringLiteral("u't'"));
        ASSERT_EQUALS(false, ::isStringLiteral("U't'"));
        ASSERT_EQUALS(false, ::isStringLiteral("L't'"));
        ASSERT_EQUALS(false, ::isStringLiteral("'test'"));
        ASSERT_EQUALS(false, ::isStringLiteral("u8'test'"));
        ASSERT_EQUALS(false, ::isStringLiteral("u'test'"));
        ASSERT_EQUALS(false, ::isStringLiteral("U'test'"));
        ASSERT_EQUALS(false, ::isStringLiteral("L'test'"));
    }

    void isCharLiteral() const {
        // empty
        ASSERT_EQUALS(false, ::isCharLiteral(""));

        // no literals
        ASSERT_EQUALS(false, ::isCharLiteral("u8"));
        ASSERT_EQUALS(false, ::isCharLiteral("u"));
        ASSERT_EQUALS(false, ::isCharLiteral("U"));
        ASSERT_EQUALS(false, ::isCharLiteral("L"));

        // incomplete string literals
        ASSERT_EQUALS(false, ::isCharLiteral("\""));
        ASSERT_EQUALS(false, ::isCharLiteral("u8\""));
        ASSERT_EQUALS(false, ::isCharLiteral("u\""));
        ASSERT_EQUALS(false, ::isCharLiteral("U\""));
        ASSERT_EQUALS(false, ::isCharLiteral("L\""));

        // valid string literals
        ASSERT_EQUALS(false, ::isCharLiteral("\"\""));
        ASSERT_EQUALS(false, ::isCharLiteral("u8\"\""));
        ASSERT_EQUALS(false, ::isCharLiteral("u\"\""));
        ASSERT_EQUALS(false, ::isCharLiteral("U\"\""));
        ASSERT_EQUALS(false, ::isCharLiteral("L\"\""));
        ASSERT_EQUALS(false, ::isCharLiteral("\"t\""));
        ASSERT_EQUALS(false, ::isCharLiteral("u8\"t\""));
        ASSERT_EQUALS(false, ::isCharLiteral("u\"t\""));
        ASSERT_EQUALS(false, ::isCharLiteral("U\"t\""));
        ASSERT_EQUALS(false, ::isCharLiteral("L\"t\""));
        ASSERT_EQUALS(false, ::isCharLiteral("\"test\""));
        ASSERT_EQUALS(false, ::isCharLiteral("u8\"test\""));
        ASSERT_EQUALS(false, ::isCharLiteral("u\"test\""));
        ASSERT_EQUALS(false, ::isCharLiteral("U\"test\""));
        ASSERT_EQUALS(false, ::isCharLiteral("L\"test\""));

        // incomplete char literals
        ASSERT_EQUALS(false, ::isCharLiteral("'"));
        ASSERT_EQUALS(false, ::isCharLiteral("u8'"));
        ASSERT_EQUALS(false, ::isCharLiteral("u'"));
        ASSERT_EQUALS(false, ::isCharLiteral("U'"));
        ASSERT_EQUALS(false, ::isCharLiteral("L'"));

        // valid char literals
        ASSERT_EQUALS(true, ::isCharLiteral("'t'"));
        ASSERT_EQUALS(true, ::isCharLiteral("u8't'"));
        ASSERT_EQUALS(true, ::isCharLiteral("u't'"));
        ASSERT_EQUALS(true, ::isCharLiteral("U't'"));
        ASSERT_EQUALS(true, ::isCharLiteral("L't'"));
        ASSERT_EQUALS(true, ::isCharLiteral("'test'"));
        ASSERT_EQUALS(true, ::isCharLiteral("u8'test'"));
        ASSERT_EQUALS(true, ::isCharLiteral("u'test'"));
        ASSERT_EQUALS(true, ::isCharLiteral("U'test'"));
        ASSERT_EQUALS(true, ::isCharLiteral("L'test'"));
    }
};

REGISTER_TEST(TestUtils)
