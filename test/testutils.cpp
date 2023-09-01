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

#include "fixture.h"
#include "utils.h"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

class TestUtils : public TestFixture {
public:
    TestUtils() : TestFixture("TestUtils") {}

private:
    void run() override {
        TEST_CASE(isValidGlobPattern);
        TEST_CASE(matchglob);
        TEST_CASE(isStringLiteral);
        TEST_CASE(isCharLiteral);
        TEST_CASE(strToInt);
        TEST_CASE(id_string);
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

    void strToInt() {
        ASSERT_EQUALS(1, ::strToInt<int>("1"));
        ASSERT_EQUALS(-1, ::strToInt<int>("-1"));
        ASSERT_EQUALS(1, ::strToInt<std::size_t>("1"));
        ASSERT_THROW_EQUALS_2(::strToInt<int>(""), std::runtime_error, "converting '' to integer failed - not an integer");
        ASSERT_THROW_EQUALS_2(::strToInt<std::size_t>(""), std::runtime_error, "converting '' to integer failed - not an integer");
        ASSERT_THROW_EQUALS_2(::strToInt<int>(" "), std::runtime_error, "converting ' ' to integer failed - not an integer");
        ASSERT_THROW_EQUALS_2(::strToInt<std::size_t>(" "), std::runtime_error, "converting ' ' to integer failed - not an integer");
        ASSERT_THROW_EQUALS_2(::strToInt<unsigned int>("-1"), std::runtime_error, "converting '-1' to integer failed - needs to be positive");
        ASSERT_THROW_EQUALS_2(::strToInt<int>("1ms"), std::runtime_error, "converting '1ms' to integer failed - not an integer");
        ASSERT_THROW_EQUALS_2(::strToInt<int>("1.0"), std::runtime_error, "converting '1.0' to integer failed - not an integer");
        ASSERT_THROW_EQUALS_2(::strToInt<int>("one"), std::runtime_error, "converting 'one' to integer failed - not an integer");
        ASSERT_THROW_EQUALS_2(::strToInt<unsigned int>("1ms"), std::runtime_error, "converting '1ms' to integer failed - not an integer");
        ASSERT_THROW_EQUALS_2(::strToInt<unsigned int>("1.0"), std::runtime_error, "converting '1.0' to integer failed - not an integer");
        ASSERT_THROW_EQUALS_2(::strToInt<unsigned int>("one"), std::runtime_error, "converting 'one' to integer failed - not an integer");
        ASSERT_THROW_EQUALS_2(::strToInt<int>(std::to_string(static_cast<int64_t>(std::numeric_limits<int>::max()) + 1)), std::runtime_error, "converting '2147483648' to integer failed - out of range (limits)");
        ASSERT_THROW_EQUALS_2(::strToInt<int>(std::to_string(static_cast<int64_t>(std::numeric_limits<int>::min()) - 1)), std::runtime_error, "converting '-2147483649' to integer failed - out of range (limits)");
        ASSERT_THROW_EQUALS_2(::strToInt<int8_t>(std::to_string(static_cast<int64_t>(std::numeric_limits<int8_t>::max()) + 1)), std::runtime_error, "converting '128' to integer failed - out of range (limits)");
        ASSERT_THROW_EQUALS_2(::strToInt<int8_t>(std::to_string(static_cast<int64_t>(std::numeric_limits<int8_t>::min()) - 1)), std::runtime_error, "converting '-129' to integer failed - out of range (limits)");
        ASSERT_THROW_EQUALS_2(::strToInt<unsigned int>(std::to_string(static_cast<uint64_t>(std::numeric_limits<unsigned int>::max()) + 1)), std::runtime_error, "converting '4294967296' to integer failed - out of range (limits)");
        ASSERT_THROW_EQUALS_2(::strToInt<int>("9223372036854775808"), std::runtime_error, "converting '9223372036854775808' to integer failed - out of range (stoll)"); // LLONG_MAX + 1
        ASSERT_THROW_EQUALS_2(::strToInt<std::size_t>("18446744073709551616"), std::runtime_error, "converting '18446744073709551616' to integer failed - out of range (stoull)"); // ULLONG_MAX + 1

        {
            long tmp;
            ASSERT(::strToInt("1", tmp));
            ASSERT_EQUALS(1, tmp);
        }
        {
            long tmp;
            ASSERT(::strToInt("-1", tmp));
            ASSERT_EQUALS(-1, tmp);
        }
        {
            std::size_t tmp;
            ASSERT(::strToInt("1", tmp));
        }
        {
            std::size_t tmp;
            ASSERT(!::strToInt("-1", tmp));
        }
        {
            long tmp;
            ASSERT(!::strToInt("1ms", tmp));
        }
        {
            unsigned long tmp;
            ASSERT(!::strToInt("1ms", tmp));
        }
        {
            long tmp;
            ASSERT(!::strToInt("", tmp));
        }
        {
            unsigned long tmp;
            ASSERT(!::strToInt("", tmp));
        }
        {
            long tmp;
            ASSERT(!::strToInt(" ", tmp));
        }
        {
            unsigned long tmp;
            ASSERT(!::strToInt(" ", tmp));
        }

        {
            long tmp;
            std::string err;
            ASSERT(::strToInt("1", tmp, &err));
            ASSERT_EQUALS(1, tmp);
            ASSERT_EQUALS("", err);
        }
        {
            std::size_t tmp;
            std::string err;
            ASSERT(::strToInt("1", tmp, &err));
            ASSERT_EQUALS(1, tmp);
            ASSERT_EQUALS("", err);
        }
        {
            unsigned long tmp;
            std::string err;
            ASSERT(!::strToInt("-1", tmp, &err));
            ASSERT_EQUALS("needs to be positive", err);
        }
        {
            long tmp;
            std::string err;
            ASSERT(!::strToInt("1ms", tmp, &err));
            ASSERT_EQUALS("not an integer", err);
        }
        {
            long tmp;
            std::string err;
            ASSERT(!::strToInt("1.0", tmp, &err));
            ASSERT_EQUALS("not an integer", err);
        }
        {
            long tmp;
            std::string err;
            ASSERT(!::strToInt("one", tmp, &err));
            ASSERT_EQUALS("not an integer", err);
        }
        {
            std::size_t tmp;
            std::string err;
            ASSERT(!::strToInt("1ms", tmp, &err));
            ASSERT_EQUALS("not an integer", err);
        }
        {
            long tmp;
            std::string err;
            ASSERT(!::strToInt("9223372036854775808", tmp, &err)); // LLONG_MAX + 1
            ASSERT_EQUALS("out of range (stoll)", err);
        }
        {
            int tmp;
            std::string err;
            ASSERT(!::strToInt(std::to_string(static_cast<int64_t>(std::numeric_limits<int>::max()) + 1), tmp, &err));
            ASSERT_EQUALS("out of range (limits)", err);
        }
        {
            int tmp;
            std::string err;
            ASSERT(!::strToInt(std::to_string(static_cast<int64_t>(std::numeric_limits<int>::min()) - 1), tmp, &err));
            ASSERT_EQUALS("out of range (limits)", err);
        }
        {
            int8_t tmp;
            std::string err;
            ASSERT(!::strToInt(std::to_string(static_cast<int64_t>(std::numeric_limits<int8_t>::max()) + 1), tmp, &err));
            ASSERT_EQUALS("out of range (limits)", err);
        }
        {
            int8_t tmp;
            std::string err;
            ASSERT(!::strToInt(std::to_string(static_cast<int64_t>(std::numeric_limits<int8_t>::min()) - 1), tmp, &err));
            ASSERT_EQUALS("out of range (limits)", err);
        }
        {
            std::size_t tmp;
            std::string err;
            ASSERT(!::strToInt("18446744073709551616", tmp, &err)); // ULLONG_MAX + 1
            ASSERT_EQUALS("out of range (stoull)", err);
        }
    }

    void id_string() const
    {
        ASSERT_EQUALS("0", id_string_i(0));
        ASSERT_EQUALS("f1", id_string_i(0xF1));
        ASSERT_EQUALS("123", id_string_i(0x123));
        ASSERT_EQUALS("1230", id_string_i(0x1230));
        ASSERT_EQUALS(std::string(8,'f'), id_string_i(0xffffffffU));
        if (sizeof(void*) == 8) {
            ASSERT_EQUALS(std::string(16,'f'), id_string_i(~0ULL));
        }
    }
};

REGISTER_TEST(TestUtils)
