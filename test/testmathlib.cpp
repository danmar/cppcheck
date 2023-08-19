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


#include "errortypes.h"
#include "mathlib.h"
#include "fixture.h"

#include <limits>
#include <string>



class TestMathLib : public TestFixture {
public:
    TestMathLib() : TestFixture("TestMathLib") {}

private:

    void run() override {
        TEST_CASE(isint);
        TEST_CASE(isbin);
        TEST_CASE(isdec);
        TEST_CASE(isoct);
        TEST_CASE(isFloatHex);
        TEST_CASE(isIntHex);
        TEST_CASE(isValidIntegerSuffix);
        TEST_CASE(isnegative);
        TEST_CASE(ispositive);
        TEST_CASE(isFloat);
        TEST_CASE(isDecimalFloat);
        TEST_CASE(isGreater);
        TEST_CASE(isGreaterEqual);
        TEST_CASE(isEqual);
        TEST_CASE(isNotEqual);
        TEST_CASE(isLess);
        TEST_CASE(isLessEqual);
        TEST_CASE(calculate);
        TEST_CASE(calculate1);
        TEST_CASE(typesuffix);
        TEST_CASE(toLongNumber);
        TEST_CASE(toULongNumber);
        TEST_CASE(toDoubleNumber);
        TEST_CASE(naninf);
        TEST_CASE(isNullValue);
        TEST_CASE(sin);
        TEST_CASE(cos);
        TEST_CASE(tan);
        TEST_CASE(abs);
        TEST_CASE(toString);
        TEST_CASE(CPP14DigitSeparators);
    }

    void isGreater() const {
        ASSERT_EQUALS(true,  MathLib::isGreater("1.0", "0.001"));
        ASSERT_EQUALS(false, MathLib::isGreater("-1.0", "0.001"));
    }

    void isGreaterEqual() const {
        ASSERT_EQUALS(true,  MathLib::isGreaterEqual("1.00", "1.0"));
        ASSERT_EQUALS(true,  MathLib::isGreaterEqual("1.001", "1.0"));
        ASSERT_EQUALS(true,  MathLib::isGreaterEqual("1.0", "0.001"));
        ASSERT_EQUALS(false, MathLib::isGreaterEqual("-1.0", "0.001"));
    }

    void isEqual() const {
        ASSERT_EQUALS(true,  MathLib::isEqual("1.0", "1.0"));
        ASSERT_EQUALS(false, MathLib::isEqual("1.", "1.01"));
        ASSERT_EQUALS(true,  MathLib::isEqual("0.1","1.0E-1"));
    }

    void isNotEqual() const {
        ASSERT_EQUALS(false, MathLib::isNotEqual("1.0", "1.0"));
        ASSERT_EQUALS(true,  MathLib::isNotEqual("1.", "1.01"));
    }

    void isLess() const {
        ASSERT_EQUALS(false, MathLib::isLess("1.0", "0.001"));
        ASSERT_EQUALS(true,  MathLib::isLess("-1.0", "0.001"));
    }

    void isLessEqual() const {
        ASSERT_EQUALS(true,  MathLib::isLessEqual("1.00", "1.0"));
        ASSERT_EQUALS(false, MathLib::isLessEqual("1.001", "1.0"));
        ASSERT_EQUALS(false, MathLib::isLessEqual("1.0", "0.001"));
        ASSERT_EQUALS(true,  MathLib::isLessEqual("-1.0", "0.001"));
    }

    void calculate() const {
        // addition
        ASSERT_EQUALS("256",  MathLib::add("0xff", "1"));
        ASSERT_EQUALS("249",  MathLib::add("250", "-1"));
        ASSERT_EQUALS("251",  MathLib::add("250", "1"));
        ASSERT_EQUALS("-2.0", MathLib::add("-1.", "-1"));
        ASSERT_EQUALS("-1",   MathLib::add("0", "-1"));
        ASSERT_EQUALS("1",    MathLib::add("1", "0"));
        ASSERT_EQUALS("0.0",  MathLib::add("0", "0."));
        ASSERT_EQUALS("1.00000001", MathLib::add("1", "0.00000001")); // #4016
        ASSERT_EQUALS("30666.22",   MathLib::add("30666.22", "0.0")); // #4068

        // subtraction
        ASSERT_EQUALS("254", MathLib::subtract("0xff", "1"));
        ASSERT_EQUALS("251", MathLib::subtract("250", "-1"));
        ASSERT_EQUALS("249", MathLib::subtract("250", "1"));
        ASSERT_EQUALS("0.0", MathLib::subtract("-1.", "-1"));
        ASSERT_EQUALS("1",   MathLib::subtract("0", "-1"));
        ASSERT_EQUALS("1",   MathLib::subtract("1", "0"));
        ASSERT_EQUALS("0.0", MathLib::subtract("0", "0."));
        ASSERT_EQUALS("0.99999999", MathLib::subtract("1", "0.00000001")); // #4016
        ASSERT_EQUALS("30666.22",   MathLib::subtract("30666.22", "0.0")); // #4068
        ASSERT_EQUALS("0.0", MathLib::subtract("0.0", "0.0"));

        // multiply
        ASSERT_EQUALS("-0.003",     MathLib::multiply("-1e-3", "3"));
        ASSERT_EQUALS("-11.96",     MathLib::multiply("-2.3", "5.2"));
        ASSERT_EQUALS("3000.0",     MathLib::multiply("1E3", "3"));
        ASSERT_EQUALS("3000.0",     MathLib::multiply("1E+3", "3"));
        ASSERT_EQUALS("3000.0",     MathLib::multiply("1.0E3", "3"));
        ASSERT_EQUALS("-3000.0",    MathLib::multiply("-1.0E3", "3"));
        ASSERT_EQUALS("-3000.0",    MathLib::multiply("-1.0E+3", "3"));
        ASSERT_EQUALS("0.0",        MathLib::multiply("+1.0E+3", "0"));
        ASSERT_EQUALS("2147483648", MathLib::multiply("2","1073741824"));
        ASSERT_EQUALS("536870912",  MathLib::multiply("512","1048576"));

        // divide
        ASSERT_EQUALS("1",    MathLib::divide("1", "1"));
        ASSERT_EQUALS("0",    MathLib::divide("0", "1"));
        ASSERT_EQUALS("5",    MathLib::divide("-10", "-2"));
        ASSERT_EQUALS("-2.5", MathLib::divide("-10.", "4"));
        ASSERT_EQUALS("2.5",  MathLib::divide("-10.", "-4"));
        ASSERT_EQUALS("5.0",  MathLib::divide("25.5", "5.1"));
        ASSERT_EQUALS("7.0",  MathLib::divide("21.", "3"));
        ASSERT_EQUALS("1",    MathLib::divide("3", "2"));
        ASSERT_THROW_EQUALS(MathLib::divide("123", "0"), InternalError, "Internal Error: Division by zero");  // decimal zero: throw
        ASSERT_THROW_EQUALS(MathLib::divide("123", "00"), InternalError, "Internal Error: Division by zero"); // octal zero: throw
        ASSERT_THROW_EQUALS(MathLib::divide("123", "0x0"), InternalError, "Internal Error: Division by zero"); // hex zero: throw
        MathLib::divide("123", "0.0f"); // float zero: don't throw
        MathLib::divide("123", "0.0"); // double zero: don't throw
        MathLib::divide("123", "0.0L"); // long double zero: don't throw
        ASSERT_THROW_EQUALS(MathLib::divide("-9223372036854775808", "-1"), InternalError, "Internal Error: Division overflow"); // #4520 - out of range => throw
        ASSERT_EQUALS("4611686018427387904",  MathLib::divide("-9223372036854775808", "-2")); // #6679


        // invoke for each supported action
        ASSERT_EQUALS("3", MathLib::calculate("2", "1", '+'));
        ASSERT_EQUALS("1", MathLib::calculate("2", "1", '-'));
        ASSERT_EQUALS("2", MathLib::calculate("2", "1", '*'));
        ASSERT_EQUALS("2", MathLib::calculate("2", "1", '/'));
        ASSERT_EQUALS("0", MathLib::calculate("2", "1", '%'));
        ASSERT_EQUALS("0", MathLib::calculate("1", "2", '&'));
        ASSERT_EQUALS("1", MathLib::calculate("1", "1", '|'));
        ASSERT_EQUALS("1", MathLib::calculate("0", "1", '^'));

        // Unknown action should throw exception
        ASSERT_THROW_EQUALS(MathLib::calculate("1","2",'j'),InternalError, "Unexpected action 'j' in MathLib::calculate(). Please report this to Cppcheck developers.");
    }

    void calculate1() const {
        ASSERT_EQUALS("0", MathLib::calculate("2",   "1", '%'));
        ASSERT_EQUALS("2", MathLib::calculate("12",  "5", '%'));
        ASSERT_EQUALS("1", MathLib::calculate("100", "3", '%'));

#ifndef TEST_MATHLIB_VALUE
        // floating point modulo is not defined in C/C++
        ASSERT_EQUALS("0.0",  MathLib::calculate("2.0",  "1.0",  '%'));
        ASSERT_EQUALS("12.0", MathLib::calculate("12.0", "13.0", '%'));
        ASSERT_EQUALS("1.3",  MathLib::calculate("5.3",  "2.0",  '%'));
        ASSERT_EQUALS("1.7",  MathLib::calculate("18.5", "4.2",  '%'));
        MathLib::calculate("123", "0.0", '%'); // don't throw
#endif

        ASSERT_THROW_EQUALS(MathLib::calculate("123", "0", '%'), InternalError, "Internal Error: Division by zero"); // throw

        ASSERT_EQUALS("0", MathLib::calculate("1", "1", '^'));
        ASSERT_EQUALS("3", MathLib::calculate("2", "1", '^'));
    }

    void typesuffix() const {
        ASSERT_EQUALS("2",    MathLib::add("1",    "1"));
        ASSERT_EQUALS("2U",   MathLib::add("1U",   "1"));
        ASSERT_EQUALS("2L",   MathLib::add("1L",   "1"));
        ASSERT_EQUALS("2UL",  MathLib::add("1UL",  "1"));
        ASSERT_EQUALS("2LL",  MathLib::add("1LL",  "1"));
        ASSERT_EQUALS("2LL",  MathLib::add("1i64", "1"));
        ASSERT_EQUALS("2ULL", MathLib::add("1ULL", "1"));
        ASSERT_EQUALS("2ULL", MathLib::add("1ui64","1"));

        ASSERT_EQUALS("2U",   MathLib::add("1",    "1U"));
        ASSERT_EQUALS("2U",   MathLib::add("1U",   "1U"));
        ASSERT_EQUALS("2L",   MathLib::add("1L",   "1U"));
        ASSERT_EQUALS("2UL",  MathLib::add("1UL",  "1U"));
        ASSERT_EQUALS("2LL",  MathLib::add("1LL",  "1U"));
        ASSERT_EQUALS("2ULL", MathLib::add("1ULL", "1U"));

        ASSERT_EQUALS("2L",   MathLib::add("1",    "1L"));
        ASSERT_EQUALS("2L",   MathLib::add("1U",   "1L"));
        ASSERT_EQUALS("2L",   MathLib::add("1L",   "1L"));
        ASSERT_EQUALS("2UL",  MathLib::add("1UL",  "1L"));
        ASSERT_EQUALS("2LL",  MathLib::add("1LL",  "1L"));
        ASSERT_EQUALS("2ULL", MathLib::add("1ULL", "1L"));

        ASSERT_EQUALS("2UL",  MathLib::add("1",    "1UL"));
        ASSERT_EQUALS("2UL",  MathLib::add("1U",   "1UL"));
        ASSERT_EQUALS("2UL",  MathLib::add("1L",   "1UL"));
        ASSERT_EQUALS("2UL",  MathLib::add("1UL",  "1UL"));
        ASSERT_EQUALS("2LL",  MathLib::add("1LL",  "1UL"));
        ASSERT_EQUALS("2ULL", MathLib::add("1ULL", "1UL"));

        ASSERT_EQUALS("2UL",  MathLib::add("1",    "1LU"));
        ASSERT_EQUALS("2UL",  MathLib::add("1U",   "1LU"));
        ASSERT_EQUALS("2UL",  MathLib::add("1L",   "1LU"));
        ASSERT_EQUALS("2UL",  MathLib::add("1UL",  "1LU"));
        ASSERT_EQUALS("2LL",  MathLib::add("1LL",  "1LU"));
        ASSERT_EQUALS("2ULL", MathLib::add("1ULL", "1LU"));

        ASSERT_EQUALS("2LL",  MathLib::add("1",    "1LL"));
        ASSERT_EQUALS("2LL",  MathLib::add("1U",   "1LL"));
        ASSERT_EQUALS("2LL",  MathLib::add("1L",   "1LL"));
        ASSERT_EQUALS("2LL",  MathLib::add("1UL",  "1LL"));
        ASSERT_EQUALS("2LL",  MathLib::add("1LL",  "1LL"));
        ASSERT_EQUALS("2ULL", MathLib::add("1ULL", "1LL"));

        ASSERT_EQUALS("2ULL",  MathLib::add("1",    "1ULL"));
        ASSERT_EQUALS("2ULL",  MathLib::add("1U",   "1ULL"));
        ASSERT_EQUALS("2ULL",  MathLib::add("1L",   "1ULL"));
        ASSERT_EQUALS("2ULL",  MathLib::add("1UL",  "1ULL"));
        ASSERT_EQUALS("2ULL",  MathLib::add("1LL",  "1ULL"));
        ASSERT_EQUALS("2ULL",  MathLib::add("1ULL", "1ULL"));

        ASSERT_EQUALS("2ULL",  MathLib::add("1",    "1LLU"));
        ASSERT_EQUALS("2ULL",  MathLib::add("1U",   "1LLU"));
        ASSERT_EQUALS("2ULL",  MathLib::add("1L",   "1LLU"));
        ASSERT_EQUALS("2ULL",  MathLib::add("1UL",  "1LLU"));
        ASSERT_EQUALS("2ULL",  MathLib::add("1LL",  "1LLU"));
        ASSERT_EQUALS("2ULL",  MathLib::add("1ULL", "1LLU"));
    }

    void toLongNumber() const {
        // zero input
        ASSERT_EQUALS(0, MathLib::toLongNumber("0"));
        ASSERT_EQUALS(0, MathLib::toLongNumber("-0"));
        ASSERT_EQUALS(0, MathLib::toLongNumber("+0"));
        ASSERT_EQUALS(0, MathLib::toLongNumber("0L"));
        ASSERT_EQUALS(0, MathLib::toLongNumber("0l"));
        ASSERT_EQUALS(0, MathLib::toLongNumber("0LL"));
        ASSERT_EQUALS(0, MathLib::toLongNumber("0ll"));
        ASSERT_EQUALS(0, MathLib::toLongNumber("0U"));
        ASSERT_EQUALS(0, MathLib::toLongNumber("0u"));
        ASSERT_EQUALS(0, MathLib::toLongNumber("0UL"));
        ASSERT_EQUALS(0, MathLib::toLongNumber("0ul"));
        ASSERT_EQUALS(0, MathLib::toLongNumber("0ULL"));
        ASSERT_EQUALS(0, MathLib::toLongNumber("0ull"));
        ASSERT_EQUALS(0, MathLib::toLongNumber("0i64")); // Visual Studio-specific
        ASSERT_EQUALS(0, MathLib::toLongNumber("0ui64")); // Visual Studio-specific

        // TODO: needs to fail
        //ASSERT_EQUALS(0, MathLib::toLongNumber("0lll"));
        //ASSERT_EQUALS(0, MathLib::toLongNumber("0uu"));

        ASSERT_EQUALS(1U,     MathLib::toLongNumber("1U"));
        ASSERT_EQUALS(10000U, MathLib::toLongNumber("1e4"));
        ASSERT_EQUALS(10000U, MathLib::toLongNumber("1e4"));
        ASSERT_EQUALS(0xFF00000000000000UL, MathLib::toLongNumber("0xFF00000000000000UL"));
        ASSERT_EQUALS(0x0A00000000000000UL, MathLib::toLongNumber("0x0A00000000000000UL"));

        // from hex
        ASSERT_EQUALS(0,      MathLib::toLongNumber("0x0"));
        ASSERT_EQUALS(0,      MathLib::toLongNumber("-0x0"));
        ASSERT_EQUALS(0,      MathLib::toLongNumber("+0x0"));
        ASSERT_EQUALS(10,     MathLib::toLongNumber("0xa"));
        ASSERT_EQUALS(10995,  MathLib::toLongNumber("0x2AF3"));
        ASSERT_EQUALS(-10,    MathLib::toLongNumber("-0xa"));
        ASSERT_EQUALS(-10995, MathLib::toLongNumber("-0x2AF3"));
        ASSERT_EQUALS(10,     MathLib::toLongNumber("+0xa"));
        ASSERT_EQUALS(10995,  MathLib::toLongNumber("+0x2AF3"));

        // from octal
        ASSERT_EQUALS(8,    MathLib::toLongNumber("010"));
        ASSERT_EQUALS(8,    MathLib::toLongNumber("+010"));
        ASSERT_EQUALS(-8,   MathLib::toLongNumber("-010"));
        ASSERT_EQUALS(125,  MathLib::toLongNumber("0175"));
        ASSERT_EQUALS(125,  MathLib::toLongNumber("+0175"));
        ASSERT_EQUALS(-125, MathLib::toLongNumber("-0175"));

        // from binary
        ASSERT_EQUALS(0,    MathLib::toLongNumber("0b0"));
        ASSERT_EQUALS(1,    MathLib::toLongNumber("0b1"));
        ASSERT_EQUALS(1,    MathLib::toLongNumber("0b1U"));
        ASSERT_EQUALS(1,    MathLib::toLongNumber("0b1L"));
        ASSERT_EQUALS(1,    MathLib::toLongNumber("0b1LU"));
        ASSERT_EQUALS(1,    MathLib::toLongNumber("0b1LL"));
        ASSERT_EQUALS(1,    MathLib::toLongNumber("0b1LLU"));
        ASSERT_EQUALS(1,    MathLib::toLongNumber("+0b1"));
        ASSERT_EQUALS(-1,   MathLib::toLongNumber("-0b1"));
        ASSERT_EQUALS(9U,   MathLib::toLongNumber("011"));
        ASSERT_EQUALS(5U,   MathLib::toLongNumber("0b101"));
        ASSERT_EQUALS(215,  MathLib::toLongNumber("0b11010111"));
        ASSERT_EQUALS(-215, MathLib::toLongNumber("-0b11010111"));
        ASSERT_EQUALS(215,  MathLib::toLongNumber("0B11010111"));

        // from base 10
        ASSERT_EQUALS(10,  MathLib::toLongNumber("10"));
        ASSERT_EQUALS(10,  MathLib::toLongNumber("10."));
        ASSERT_EQUALS(10,  MathLib::toLongNumber("10.0"));
        ASSERT_EQUALS(100, MathLib::toLongNumber("10E+1"));
        ASSERT_EQUALS(1,   MathLib::toLongNumber("10E-1"));
        ASSERT_EQUALS(100, MathLib::toLongNumber("+10E+1"));
        ASSERT_EQUALS(-1,  MathLib::toLongNumber("-10E-1"));
        ASSERT_EQUALS(100, MathLib::toLongNumber("+10.E+1"));
        ASSERT_EQUALS(-1,  MathLib::toLongNumber("-10.E-1"));
        ASSERT_EQUALS(100, MathLib::toLongNumber("+10.0E+1"));
        ASSERT_EQUALS(-1,  MathLib::toLongNumber("-10.0E-1"));

        // from char
        ASSERT_EQUALS((int)('A'),    MathLib::toLongNumber("'A'"));
        ASSERT_EQUALS((int)('\x10'), MathLib::toLongNumber("'\\x10'"));
        ASSERT_EQUALS((int)('\100'), MathLib::toLongNumber("'\\100'"));
        ASSERT_EQUALS((int)('\200'), MathLib::toLongNumber("'\\200'"));
        ASSERT_EQUALS((int)(L'A'),   MathLib::toLongNumber("L'A'"));

        ASSERT_EQUALS(-8552249625308161526, MathLib::toLongNumber("0x89504e470d0a1a0a"));
        ASSERT_EQUALS(-8481036456200365558, MathLib::toLongNumber("0x8a4d4e470d0a1a0a"));

        // from long long
        /*
         * ASSERT_EQUALS(0xFF00000000000000LL, MathLib::toLongNumber("0xFF00000000000000LL"));
         * This does not work in a portable way!
         * While it succeeds on 32bit Visual Studio it fails on Linux 64bit because it is greater than 0x7FFFFFFFFFFFFFFF (=LLONG_MAX)
         */

        ASSERT_EQUALS(0x0A00000000000000LL, MathLib::toLongNumber("0x0A00000000000000LL"));

        // min/max numeric limits
        ASSERT_EQUALS(std::numeric_limits<long long>::min(), MathLib::toLongNumber(std::to_string(std::numeric_limits<long long>::min())));
        ASSERT_EQUALS(std::numeric_limits<long long>::max(), MathLib::toLongNumber(std::to_string(std::numeric_limits<long long>::max())));
        ASSERT_EQUALS(std::numeric_limits<unsigned long long>::min(), MathLib::toLongNumber(std::to_string(std::numeric_limits<unsigned long long>::min())));
        ASSERT_EQUALS(std::numeric_limits<unsigned long long>::max(), MathLib::toLongNumber(std::to_string(std::numeric_limits<unsigned long long>::max())));

        // min/max and out-of-bounds - hex
        {
            const MathLib::bigint i = 0xFFFFFFFFFFFFFFFF;
            ASSERT_EQUALS(i, MathLib::toLongNumber(std::to_string(i)));
            ASSERT_EQUALS(i, MathLib::toLongNumber("0xFFFFFFFFFFFFFFFF"));
        }
        {
            const MathLib::bigint i = -0xFFFFFFFFFFFFFFFF;
            ASSERT_EQUALS(i, MathLib::toLongNumber(std::to_string(i)));
            ASSERT_EQUALS(i, MathLib::toLongNumber("-0xFFFFFFFFFFFFFFFF"));
        }

        ASSERT_THROW_EQUALS(MathLib::toLongNumber("0x10000000000000000"), InternalError, "Internal Error. MathLib::toLongNumber: out_of_range: 0x10000000000000000");
        ASSERT_THROW_EQUALS(MathLib::toLongNumber("-0x10000000000000000"), InternalError, "Internal Error. MathLib::toLongNumber: out_of_range: -0x10000000000000000");

        // min/max and out-of-bounds - octal
        {
            const MathLib::bigint i = 01777777777777777777777;
            ASSERT_EQUALS(i, MathLib::toLongNumber(std::to_string(i)));
            ASSERT_EQUALS(i, MathLib::toLongNumber("01777777777777777777777"));
        }
        {
            const MathLib::bigint i = -01777777777777777777777;
            ASSERT_EQUALS(i, MathLib::toLongNumber(std::to_string(i)));
            ASSERT_EQUALS(i, MathLib::toLongNumber("-01777777777777777777777"));
        }

        ASSERT_THROW_EQUALS(MathLib::toLongNumber("02000000000000000000000"), InternalError, "Internal Error. MathLib::toLongNumber: out_of_range: 02000000000000000000000");
        ASSERT_THROW_EQUALS(MathLib::toLongNumber("-02000000000000000000000"), InternalError, "Internal Error. MathLib::toLongNumber: out_of_range: -02000000000000000000000");

        // min/max and out-of-bounds - decimal
        SUPPRESS_WARNING_CLANG_PUSH("-Wimplicitly-unsigned-literal")
        SUPPRESS_WARNING_GCC_PUSH("-Woverflow")
        {
            const MathLib::bigint i = 18446744073709551615;
            ASSERT_EQUALS(i, MathLib::toLongNumber(std::to_string(i)));
            ASSERT_EQUALS(i, MathLib::toLongNumber("18446744073709551615"));
        }
        {
            const MathLib::bigint i = -18446744073709551615;
            ASSERT_EQUALS(i, MathLib::toLongNumber(std::to_string(i)));
            ASSERT_EQUALS(i, MathLib::toLongNumber("-18446744073709551615"));
        }
        SUPPRESS_WARNING_GCC_POP
        SUPPRESS_WARNING_CLANG_POP

            ASSERT_THROW_EQUALS(MathLib::toLongNumber("18446744073709551616"), InternalError, "Internal Error. MathLib::toLongNumber: out_of_range: 18446744073709551616");
        ASSERT_THROW_EQUALS(MathLib::toLongNumber("-18446744073709551616"), InternalError, "Internal Error. MathLib::toLongNumber: out_of_range: -18446744073709551616");

        ASSERT_THROW_EQUALS(MathLib::toLongNumber("invalid"), InternalError, "Internal Error. MathLib::toLongNumber: invalid_argument: invalid");

        ASSERT_THROW_EQUALS(MathLib::toLongNumber("1invalid"), InternalError, "Internal Error. MathLib::toLongNumber: input was not completely consumed: 1invalid");
        ASSERT_THROW_EQUALS(MathLib::toLongNumber("1 invalid"), InternalError, "Internal Error. MathLib::toLongNumber: input was not completely consumed: 1 invalid");

        // TODO: test binary
        // TODO: test floating point

        // TODO: test with 128-bit values
    }

    void toULongNumber() const {
        // zero input
        ASSERT_EQUALS(0, MathLib::toULongNumber("0"));
        ASSERT_EQUALS(0, MathLib::toULongNumber("-0"));
        ASSERT_EQUALS(0, MathLib::toULongNumber("+0"));
        ASSERT_EQUALS(0, MathLib::toULongNumber("0L"));
        ASSERT_EQUALS(0, MathLib::toULongNumber("0l"));
        ASSERT_EQUALS(0, MathLib::toULongNumber("0LL"));
        ASSERT_EQUALS(0, MathLib::toULongNumber("0ll"));
        ASSERT_EQUALS(0, MathLib::toULongNumber("0U"));
        ASSERT_EQUALS(0, MathLib::toULongNumber("0u"));
        ASSERT_EQUALS(0, MathLib::toULongNumber("0UL"));
        ASSERT_EQUALS(0, MathLib::toULongNumber("0ul"));
        ASSERT_EQUALS(0, MathLib::toULongNumber("0ULL"));
        ASSERT_EQUALS(0, MathLib::toULongNumber("0ull"));
        ASSERT_EQUALS(0, MathLib::toULongNumber("0i64")); // Visual Studio-specific
        ASSERT_EQUALS(0, MathLib::toULongNumber("0ui64")); // Visual Studio-specific

        // TODO: needs to fail
        //ASSERT_EQUALS(0, MathLib::toULongNumber("0lll"));
        //ASSERT_EQUALS(0, MathLib::toULongNumber("0uu"));

        ASSERT_EQUALS(1U,     MathLib::toULongNumber("1U"));
        ASSERT_EQUALS(10000U, MathLib::toULongNumber("1e4"));
        ASSERT_EQUALS(10000U, MathLib::toULongNumber("1e4"));
        ASSERT_EQUALS(0xFF00000000000000UL, MathLib::toULongNumber("0xFF00000000000000UL"));
        ASSERT_EQUALS(0x0A00000000000000UL, MathLib::toULongNumber("0x0A00000000000000UL"));

        // from hex
        ASSERT_EQUALS(0,      MathLib::toULongNumber("0x0"));
        ASSERT_EQUALS(0,      MathLib::toULongNumber("-0x0"));
        ASSERT_EQUALS(0,      MathLib::toULongNumber("+0x0"));
        ASSERT_EQUALS(10,     MathLib::toULongNumber("0xa"));
        ASSERT_EQUALS(10995,  MathLib::toULongNumber("0x2AF3"));
        ASSERT_EQUALS(-10,    MathLib::toULongNumber("-0xa"));
        ASSERT_EQUALS(-10995, MathLib::toULongNumber("-0x2AF3"));
        ASSERT_EQUALS(10,     MathLib::toULongNumber("+0xa"));
        ASSERT_EQUALS(10995,  MathLib::toULongNumber("+0x2AF3"));

        // from octal
        ASSERT_EQUALS(8,    MathLib::toULongNumber("010"));
        ASSERT_EQUALS(8,    MathLib::toULongNumber("+010"));
        ASSERT_EQUALS(-8,   MathLib::toULongNumber("-010"));
        ASSERT_EQUALS(125,  MathLib::toULongNumber("0175"));
        ASSERT_EQUALS(125,  MathLib::toULongNumber("+0175"));
        ASSERT_EQUALS(-125, MathLib::toULongNumber("-0175"));

        // from binary
        ASSERT_EQUALS(0,    MathLib::toULongNumber("0b0"));
        ASSERT_EQUALS(1,    MathLib::toULongNumber("0b1"));
        ASSERT_EQUALS(1,    MathLib::toULongNumber("0b1U"));
        ASSERT_EQUALS(1,    MathLib::toULongNumber("0b1L"));
        ASSERT_EQUALS(1,    MathLib::toULongNumber("0b1LU"));
        ASSERT_EQUALS(1,    MathLib::toULongNumber("0b1LL"));
        ASSERT_EQUALS(1,    MathLib::toULongNumber("0b1LLU"));
        ASSERT_EQUALS(1,    MathLib::toULongNumber("+0b1"));
        ASSERT_EQUALS(-1,   MathLib::toULongNumber("-0b1"));
        ASSERT_EQUALS(9U,   MathLib::toULongNumber("011"));
        ASSERT_EQUALS(5U,   MathLib::toULongNumber("0b101"));
        ASSERT_EQUALS(215,  MathLib::toULongNumber("0b11010111"));
        ASSERT_EQUALS(-215, MathLib::toULongNumber("-0b11010111"));
        ASSERT_EQUALS(215,  MathLib::toULongNumber("0B11010111"));

        // from base 10
        ASSERT_EQUALS(10,  MathLib::toULongNumber("10"));
        ASSERT_EQUALS(10,  MathLib::toULongNumber("10."));
        ASSERT_EQUALS(10,  MathLib::toULongNumber("10.0"));
        ASSERT_EQUALS(100, MathLib::toULongNumber("10E+1"));
        ASSERT_EQUALS(1,   MathLib::toULongNumber("10E-1"));
        ASSERT_EQUALS(100, MathLib::toULongNumber("+10E+1"));
        ASSERT_EQUALS(-1,  MathLib::toULongNumber("-10E-1"));
        ASSERT_EQUALS(100, MathLib::toULongNumber("+10.E+1"));
        ASSERT_EQUALS(-1,  MathLib::toULongNumber("-10.E-1"));
        ASSERT_EQUALS(100, MathLib::toULongNumber("+10.0E+1"));
        ASSERT_EQUALS(-1,  MathLib::toULongNumber("-10.0E-1"));

        // from char
        ASSERT_EQUALS((int)('A'),    MathLib::toULongNumber("'A'"));
        ASSERT_EQUALS((int)('\x10'), MathLib::toULongNumber("'\\x10'"));
        ASSERT_EQUALS((int)('\100'), MathLib::toULongNumber("'\\100'"));
        ASSERT_EQUALS((int)('\200'), MathLib::toULongNumber("'\\200'"));
        ASSERT_EQUALS((int)(L'A'),   MathLib::toULongNumber("L'A'"));

        ASSERT_EQUALS(9894494448401390090ULL, MathLib::toULongNumber("0x89504e470d0a1a0a"));
        ASSERT_EQUALS(9965707617509186058ULL, MathLib::toULongNumber("0x8a4d4e470d0a1a0a"));

        // from long long
        /*
         * ASSERT_EQUALS(0xFF00000000000000LL, MathLib::toULongNumber("0xFF00000000000000LL"));
         * This does not work in a portable way!
         * While it succeeds on 32bit Visual Studio it fails on Linux 64bit because it is greater than 0x7FFFFFFFFFFFFFFF (=LLONG_MAX)
         */

        ASSERT_EQUALS(0x0A00000000000000LL, MathLib::toULongNumber("0x0A00000000000000LL"));

        // min/max numeric limits
        ASSERT_EQUALS(std::numeric_limits<long long>::min(), MathLib::toULongNumber(std::to_string(std::numeric_limits<long long>::min())));
        ASSERT_EQUALS(std::numeric_limits<long long>::max(), MathLib::toULongNumber(std::to_string(std::numeric_limits<long long>::max())));
        ASSERT_EQUALS(std::numeric_limits<unsigned long long>::min(), MathLib::toULongNumber(std::to_string(std::numeric_limits<unsigned long long>::min())));
        ASSERT_EQUALS(std::numeric_limits<unsigned long long>::max(), MathLib::toULongNumber(std::to_string(std::numeric_limits<unsigned long long>::max())));

        // min/max and out-of-bounds - hex
        {
            const MathLib::biguint u = 0xFFFFFFFFFFFFFFFF;
            ASSERT_EQUALS(u, MathLib::toULongNumber(std::to_string(u)));
            ASSERT_EQUALS(u, MathLib::toULongNumber("0xFFFFFFFFFFFFFFFF"));
        }
        {
            const MathLib::biguint u = -0xFFFFFFFFFFFFFFFF;
            ASSERT_EQUALS(u, MathLib::toULongNumber(std::to_string(u)));
            ASSERT_EQUALS(u, MathLib::toULongNumber("-0xFFFFFFFFFFFFFFFF"));
        }

        ASSERT_THROW_EQUALS(MathLib::toULongNumber("0x10000000000000000"), InternalError, "Internal Error. MathLib::toULongNumber: out_of_range: 0x10000000000000000");
        ASSERT_THROW_EQUALS(MathLib::toULongNumber("-0x10000000000000000"), InternalError, "Internal Error. MathLib::toULongNumber: out_of_range: -0x10000000000000000");

        // min/max and out-of-bounds - octal
        {
            const MathLib::biguint u = 01777777777777777777777;
            ASSERT_EQUALS(u, MathLib::toULongNumber(std::to_string(u)));
            ASSERT_EQUALS(u, MathLib::toULongNumber("01777777777777777777777"));
        }
        {
            const MathLib::biguint u = -01777777777777777777777;
            ASSERT_EQUALS(u, MathLib::toULongNumber(std::to_string(u)));
            ASSERT_EQUALS(u, MathLib::toULongNumber("-01777777777777777777777"));
        }

        ASSERT_THROW_EQUALS(MathLib::toULongNumber("02000000000000000000000"), InternalError, "Internal Error. MathLib::toULongNumber: out_of_range: 02000000000000000000000");
        ASSERT_THROW_EQUALS(MathLib::toULongNumber("-02000000000000000000000"), InternalError, "Internal Error. MathLib::toULongNumber: out_of_range: -02000000000000000000000");

        // min/max and out-of-bounds - decimal
        SUPPRESS_WARNING_CLANG_PUSH("-Wimplicitly-unsigned-literal")
        SUPPRESS_WARNING_GCC_PUSH("-Woverflow")
        {
            const MathLib::biguint u = 18446744073709551615;
            ASSERT_EQUALS(u, MathLib::toULongNumber(std::to_string(u)));
            ASSERT_EQUALS(u, MathLib::toULongNumber("18446744073709551615"));
        }
        {
            const MathLib::biguint u = -18446744073709551615;
            ASSERT_EQUALS(u, MathLib::toULongNumber(std::to_string(u)));
            ASSERT_EQUALS(u, MathLib::toULongNumber("-18446744073709551615"));
        }
        SUPPRESS_WARNING_GCC_POP
        SUPPRESS_WARNING_CLANG_POP

            ASSERT_THROW_EQUALS(MathLib::toULongNumber("18446744073709551616"), InternalError, "Internal Error. MathLib::toULongNumber: out_of_range: 18446744073709551616");
        ASSERT_THROW_EQUALS(MathLib::toULongNumber("-18446744073709551616"), InternalError, "Internal Error. MathLib::toULongNumber: out_of_range: -18446744073709551616");

        ASSERT_THROW_EQUALS(MathLib::toULongNumber("invalid"), InternalError, "Internal Error. MathLib::toULongNumber: invalid_argument: invalid");

        ASSERT_THROW_EQUALS(MathLib::toULongNumber("1invalid"), InternalError, "Internal Error. MathLib::toULongNumber: input was not completely consumed: 1invalid");
        ASSERT_THROW_EQUALS(MathLib::toULongNumber("1 invalid"), InternalError, "Internal Error. MathLib::toULongNumber: input was not completely consumed: 1 invalid");

        // TODO: test binary
        // TODO: test floating point

        // TODO: test with 128-bit values
    }

    void toDoubleNumber() const {
        // float values
        ASSERT_EQUALS_DOUBLE(1.0,    MathLib::toDoubleNumber("1."),       0.001);
        ASSERT_EQUALS_DOUBLE(1.0,    MathLib::toDoubleNumber("1.f"),      0.001);
        ASSERT_EQUALS_DOUBLE(1.0,    MathLib::toDoubleNumber("1.F"),      0.001);
        ASSERT_EQUALS_DOUBLE(1.0,    MathLib::toDoubleNumber("1.l"),      0.001);
        ASSERT_EQUALS_DOUBLE(1.0,    MathLib::toDoubleNumber("1.L"),      0.001);
        ASSERT_EQUALS_DOUBLE(1.0,    MathLib::toDoubleNumber("1.0"),      0.001);
        ASSERT_EQUALS_DOUBLE(1.0,    MathLib::toDoubleNumber("1.0f"),     0.001);
        ASSERT_EQUALS_DOUBLE(1.0,    MathLib::toDoubleNumber("1.0F"),     0.001);
        ASSERT_EQUALS_DOUBLE(1.0,    MathLib::toDoubleNumber("1.0l"),     0.001);
        ASSERT_EQUALS_DOUBLE(1.0,    MathLib::toDoubleNumber("1.0L"),     0.001);
        ASSERT_EQUALS_DOUBLE(1.0,    MathLib::toDoubleNumber("0x1"),      0.001);
        ASSERT_EQUALS_DOUBLE(10.0,   MathLib::toDoubleNumber("10"),       0.001);
        ASSERT_EQUALS_DOUBLE(1000.0, MathLib::toDoubleNumber("10E+2"),    0.001);
        ASSERT_EQUALS_DOUBLE(1000.0, MathLib::toDoubleNumber("10E+2l"),   0.001);
        ASSERT_EQUALS_DOUBLE(1000.0, MathLib::toDoubleNumber("10E+2L"),   0.001);
        ASSERT_EQUALS_DOUBLE(100.0,  MathLib::toDoubleNumber("1.0E+2"),   0.001);
        ASSERT_EQUALS_DOUBLE(-100.0, MathLib::toDoubleNumber("-1.0E+2"),  0.001);
        ASSERT_EQUALS_DOUBLE(-1e+10, MathLib::toDoubleNumber("-1.0E+10"), 1);
        ASSERT_EQUALS_DOUBLE(100.0,  MathLib::toDoubleNumber("+1.0E+2"),  0.001);
        ASSERT_EQUALS_DOUBLE(1e+10,  MathLib::toDoubleNumber("+1.0E+10"), 1);
        ASSERT_EQUALS_DOUBLE(100.0,  MathLib::toDoubleNumber("1.0E+2"),   0.001);
        ASSERT_EQUALS_DOUBLE(1e+10,  MathLib::toDoubleNumber("1.0E+10"),  1);

        // valid non-float values
        ASSERT_EQUALS_DOUBLE(1.0,    MathLib::toDoubleNumber("1"),        0.001);
        ASSERT_EQUALS_DOUBLE(1.0,    MathLib::toDoubleNumber("1l"),       0.001);
        ASSERT_EQUALS_DOUBLE(1.0,    MathLib::toDoubleNumber("1L"),       0.001);
        ASSERT_EQUALS_DOUBLE(1.0,    MathLib::toDoubleNumber("1ll"),      0.001);
        ASSERT_EQUALS_DOUBLE(1.0,    MathLib::toDoubleNumber("1LL"),      0.001);
        ASSERT_EQUALS_DOUBLE(1.0,    MathLib::toDoubleNumber("1u"),       0.001);
        ASSERT_EQUALS_DOUBLE(1.0,    MathLib::toDoubleNumber("1U"),       0.001);
        ASSERT_EQUALS_DOUBLE(1.0,    MathLib::toDoubleNumber("1ul"),      0.001);
        ASSERT_EQUALS_DOUBLE(1.0,    MathLib::toDoubleNumber("1UL"),      0.001);
        ASSERT_EQUALS_DOUBLE(1.0,    MathLib::toDoubleNumber("1ULL"),     0.001);
        // TODO: make these work with libc++
#ifndef _LIBCPP_VERSION
        ASSERT_EQUALS_DOUBLE(1.0,    MathLib::toDoubleNumber("1i64"),     0.001);
        ASSERT_EQUALS_DOUBLE(1.0,    MathLib::toDoubleNumber("1I64"),     0.001);
        ASSERT_EQUALS_DOUBLE(1.0,    MathLib::toDoubleNumber("1ui64"),    0.001);
        ASSERT_EQUALS_DOUBLE(1.0,    MathLib::toDoubleNumber("1UI64"),    0.001);
        ASSERT_EQUALS_DOUBLE(1.0,    MathLib::toDoubleNumber("1I64"),     0.001);
#endif

        ASSERT_EQUALS_DOUBLE(0.0,    MathLib::toDoubleNumber("0E+0"),     0.000001);
        ASSERT_EQUALS_DOUBLE(0.0,    MathLib::toDoubleNumber("0E-0"),     0.000001);
        ASSERT_EQUALS_DOUBLE(0.0,    MathLib::toDoubleNumber("0E+00"),    0.000001);
        ASSERT_EQUALS_DOUBLE(0.0,    MathLib::toDoubleNumber("0E-00"),    0.000001);
        ASSERT_EQUALS_DOUBLE(0.0,    MathLib::toDoubleNumber("-0E+00"),   0.000001);
        ASSERT_EQUALS_DOUBLE(0.0,    MathLib::toDoubleNumber("+0E-00"),   0.000001);
        ASSERT_EQUALS_DOUBLE(0.0,    MathLib::toDoubleNumber("0"),        0.000001);
        ASSERT_EQUALS_DOUBLE(0.0,    MathLib::toDoubleNumber("0."),       0.000001);
        ASSERT_EQUALS_DOUBLE(0.0,    MathLib::toDoubleNumber("0.0"),      0.000001);
        ASSERT_EQUALS_DOUBLE(0.0,    MathLib::toDoubleNumber("-0"),       0.000001);
        ASSERT_EQUALS_DOUBLE(0.0,    MathLib::toDoubleNumber("+0"),       0.000001);
        ASSERT_EQUALS_DOUBLE(0.0,    MathLib::toDoubleNumber("-0."),      0.000001);
        ASSERT_EQUALS_DOUBLE(0.0,    MathLib::toDoubleNumber("+0."),      0.000001);
        ASSERT_EQUALS_DOUBLE(0.0,    MathLib::toDoubleNumber("-0.0"),     0.000001);
        ASSERT_EQUALS_DOUBLE(0.0,    MathLib::toDoubleNumber("+0.0"),     0.000001);
        ASSERT_EQUALS_DOUBLE('0',    MathLib::toDoubleNumber("'0'"),      0.000001);
        ASSERT_EQUALS_DOUBLE(L'0',   MathLib::toDoubleNumber("L'0'"),     0.000001);

        ASSERT_EQUALS_DOUBLE(192, MathLib::toDoubleNumber("0x0.3p10"), 0.000001);
        ASSERT_EQUALS_DOUBLE(5.42101e-20, MathLib::toDoubleNumber("0x1p-64"), 1e-20);
        ASSERT_EQUALS_DOUBLE(3.14159, MathLib::toDoubleNumber("0x1.921fb5p+1"), 0.000001);
        ASSERT_EQUALS_DOUBLE(2006, MathLib::toDoubleNumber("0x1.f58000p+10"), 0.000001);
        ASSERT_EQUALS_DOUBLE(1e-010, MathLib::toDoubleNumber("0x1.b7cdfep-34"), 0.000001);
        ASSERT_EQUALS_DOUBLE(.484375, MathLib::toDoubleNumber("0x1.fp-2"), 0.000001);
        ASSERT_EQUALS_DOUBLE(9.0, MathLib::toDoubleNumber("0x1.2P3"), 0.000001);
        ASSERT_EQUALS_DOUBLE(0.0625, MathLib::toDoubleNumber("0x.1P0"), 0.000001);

        // from char
        ASSERT_EQUALS_DOUBLE((double)('A'),    MathLib::toDoubleNumber("'A'"), 0.000001);
        ASSERT_EQUALS_DOUBLE((double)('\x10'), MathLib::toDoubleNumber("'\\x10'"), 0.000001);
        ASSERT_EQUALS_DOUBLE((double)('\100'), MathLib::toDoubleNumber("'\\100'"), 0.000001);
        ASSERT_EQUALS_DOUBLE((double)('\200'), MathLib::toDoubleNumber("'\\200'"), 0.000001);
        ASSERT_EQUALS_DOUBLE((double)(L'A'),   MathLib::toDoubleNumber("L'A'"), 0.000001);

        ASSERT_THROW_EQUALS(MathLib::toDoubleNumber("invalid"), InternalError, "Internal Error. MathLib::toDoubleNumber: conversion failed: invalid");

#ifdef _LIBCPP_VERSION
        ASSERT_THROW_EQUALS(MathLib::toDoubleNumber("1invalid"), InternalError, "Internal Error. MathLib::toDoubleNumber: conversion failed: 1invalid");
        ASSERT_THROW_EQUALS(MathLib::toDoubleNumber("1.1invalid"), InternalError, "Internal Error. MathLib::toDoubleNumber: conversion failed: 1.1invalid");
#else
        ASSERT_THROW_EQUALS(MathLib::toDoubleNumber("1invalid"), InternalError, "Internal Error. MathLib::toDoubleNumber: input was not completely consumed: 1invalid");
        ASSERT_THROW_EQUALS(MathLib::toDoubleNumber("1.1invalid"), InternalError, "Internal Error. MathLib::toDoubleNumber: input was not completely consumed: 1.1invalid");
#endif
        ASSERT_THROW_EQUALS(MathLib::toDoubleNumber("1 invalid"), InternalError, "Internal Error. MathLib::toDoubleNumber: input was not completely consumed: 1 invalid");
        ASSERT_THROW_EQUALS(MathLib::toDoubleNumber("-1e-08.0"), InternalError, "Internal Error. MathLib::toDoubleNumber: input was not completely consumed: -1e-08.0");

        // invalid suffices
#ifdef _LIBCPP_VERSION
        ASSERT_THROW_EQUALS(MathLib::toDoubleNumber("1f"), InternalError, "Internal Error. MathLib::toDoubleNumber: conversion failed: 1f");
        ASSERT_THROW_EQUALS(MathLib::toDoubleNumber("1F"), InternalError, "Internal Error. MathLib::toDoubleNumber: conversion failed: 1F");
        ASSERT_THROW_EQUALS(MathLib::toDoubleNumber("1.ff"), InternalError, "Internal Error. MathLib::toDoubleNumber: conversion failed: 1.ff");
        ASSERT_THROW_EQUALS(MathLib::toDoubleNumber("1.FF"), InternalError, "Internal Error. MathLib::toDoubleNumber: conversion failed: 1.FF");
        ASSERT_THROW_EQUALS(MathLib::toDoubleNumber("1.0ff"), InternalError, "Internal Error. MathLib::toDoubleNumber: conversion failed: 1.0ff");
        ASSERT_THROW_EQUALS(MathLib::toDoubleNumber("1.0FF"), InternalError, "Internal Error. MathLib::toDoubleNumber: conversion failed: 1.0FF");
#else
        ASSERT_THROW_EQUALS(MathLib::toDoubleNumber("1f"), InternalError, "Internal Error. MathLib::toDoubleNumber: input was not completely consumed: 1f");
        ASSERT_THROW_EQUALS(MathLib::toDoubleNumber("1F"), InternalError, "Internal Error. MathLib::toDoubleNumber: input was not completely consumed: 1F");
        ASSERT_THROW_EQUALS(MathLib::toDoubleNumber("1.ff"), InternalError, "Internal Error. MathLib::toDoubleNumber: input was not completely consumed: 1.ff");
        ASSERT_THROW_EQUALS(MathLib::toDoubleNumber("1.FF"), InternalError, "Internal Error. MathLib::toDoubleNumber: input was not completely consumed: 1.FF");
        ASSERT_THROW_EQUALS(MathLib::toDoubleNumber("1.0ff"), InternalError, "Internal Error. MathLib::toDoubleNumber: input was not completely consumed: 1.0ff");
        ASSERT_THROW_EQUALS(MathLib::toDoubleNumber("1.0FF"), InternalError, "Internal Error. MathLib::toDoubleNumber: input was not completely consumed: 1.0FF");
#endif
        // TODO: needs to fail
        //ASSERT_THROW_EQUALS(MathLib::toDoubleNumber("1.ll"), InternalError, "Internal Error. MathLib::toDoubleNumber: input was not completely consumed: 1.ll");
        //ASSERT_THROW_EQUALS(MathLib::toDoubleNumber("1.0LL"), InternalError, "Internal Error. MathLib::toDoubleNumber: input was not completely consumed: 1.0LL");
        //ASSERT_THROW_EQUALS(MathLib::toDoubleNumber("1.0ll"), InternalError, "Internal Error. MathLib::toDoubleNumber: input was not completely consumed: 1.0ll");
        //ASSERT_THROW_EQUALS(MathLib::toDoubleNumber("1.0LL"), InternalError, "Internal Error. MathLib::toDoubleNumber: input was not completely consumed: 1.0LL");

        // verify: string --> double --> string conversion
        ASSERT_EQUALS("1.0",  MathLib::toString(MathLib::toDoubleNumber("1.0f")));
        ASSERT_EQUALS("1.0",  MathLib::toString(MathLib::toDoubleNumber("1.0")));
        ASSERT_EQUALS("0.0",  MathLib::toString(MathLib::toDoubleNumber("0.0f")));
        ASSERT_EQUALS("0.0",  MathLib::toString(MathLib::toDoubleNumber("0.0")));
        ASSERT_EQUALS("-1.0", MathLib::toString(MathLib::toDoubleNumber("-1.0f")));
        ASSERT_EQUALS("-1.0", MathLib::toString(MathLib::toDoubleNumber("-1.0")));
        ASSERT_EQUALS("0.0",  MathLib::toString(MathLib::toDoubleNumber("-0.0f")));
        ASSERT_EQUALS("0.0",  MathLib::toString(MathLib::toDoubleNumber("-0.0")));
        ASSERT_EQUALS("1.0",  MathLib::toString(MathLib::toDoubleNumber("+1.0f")));
        ASSERT_EQUALS("1.0",  MathLib::toString(MathLib::toDoubleNumber("+1.0")));
        ASSERT_EQUALS("0.0",  MathLib::toString(MathLib::toDoubleNumber("+0.0f")));
        ASSERT_EQUALS("0.0",  MathLib::toString(MathLib::toDoubleNumber("+0.0")));
        ASSERT_EQUALS("0.0",  MathLib::toString(MathLib::toDoubleNumber("-0")));
        ASSERT_EQUALS("0.0",  MathLib::toString(MathLib::toDoubleNumber("-0.")));
        ASSERT_EQUALS("0.0",  MathLib::toString(MathLib::toDoubleNumber("-0.0")));
    }

    void isint() const {
        // zero tests
        ASSERT_EQUALS(true,  MathLib::isInt("0"));
        ASSERT_EQUALS(false, MathLib::isInt("0."));
        ASSERT_EQUALS(false, MathLib::isInt("0.0"));
        ASSERT_EQUALS(false, MathLib::isInt("-0."));
        ASSERT_EQUALS(false, MathLib::isInt("+0."));
        ASSERT_EQUALS(false, MathLib::isInt("-0.0"));
        ASSERT_EQUALS(false, MathLib::isInt("+0.0"));
        ASSERT_EQUALS(false, MathLib::isInt("+0.0E+1"));
        ASSERT_EQUALS(false, MathLib::isInt("+0.0E-1"));
        ASSERT_EQUALS(false, MathLib::isInt("-0.0E+1"));
        ASSERT_EQUALS(false, MathLib::isInt("-0.0E-1"));

        ASSERT_EQUALS(true,  MathLib::isInt("1"));
        ASSERT_EQUALS(true,  MathLib::isInt("-1"));
        ASSERT_EQUALS(true,  MathLib::isInt("+1"));
        ASSERT_EQUALS(false, MathLib::isInt("+1E+1"));
        ASSERT_EQUALS(false, MathLib::isInt("+1E+10000"));
        ASSERT_EQUALS(false, MathLib::isInt("-1E+1"));
        ASSERT_EQUALS(false, MathLib::isInt("-1E+10000"));
        ASSERT_EQUALS(false, MathLib::isInt("-1E-1"));
        ASSERT_EQUALS(false, MathLib::isInt("-1E-10000"));

        ASSERT_EQUALS(true,  MathLib::isInt("0xff"));
        ASSERT_EQUALS(true,  MathLib::isInt("0xa"));
        ASSERT_EQUALS(true,  MathLib::isInt("0b1000"));
        ASSERT_EQUALS(true,  MathLib::isInt("0B1000"));
        ASSERT_EQUALS(true,  MathLib::isInt("0l"));
        ASSERT_EQUALS(true,  MathLib::isInt("0L"));
        ASSERT_EQUALS(true,  MathLib::isInt("0ul"));
        ASSERT_EQUALS(true,  MathLib::isInt("0ull"));
        ASSERT_EQUALS(true,  MathLib::isInt("0llu"));
        ASSERT_EQUALS(true,  MathLib::isInt("333L"));
        ASSERT_EQUALS(true,  MathLib::isInt("330L"));
        ASSERT_EQUALS(true,  MathLib::isInt("330llu"));
        ASSERT_EQUALS(true,  MathLib::isInt("07"));
        ASSERT_EQUALS(true,  MathLib::isInt("0123"));
        ASSERT_EQUALS(false, MathLib::isInt("0.4"));
        ASSERT_EQUALS(false, MathLib::isInt("2352.3f"));
        ASSERT_EQUALS(false, MathLib::isInt("0.00004"));
        ASSERT_EQUALS(false, MathLib::isInt("2352.00001f"));
        ASSERT_EQUALS(false, MathLib::isInt(".4"));
        ASSERT_EQUALS(false, MathLib::isInt("1.0E+1"));
        ASSERT_EQUALS(false, MathLib::isInt("1.0E-1"));
        ASSERT_EQUALS(false, MathLib::isInt("-1.0E+1"));
        ASSERT_EQUALS(false, MathLib::isInt("+1.0E-1"));
        ASSERT_EQUALS(false, MathLib::isInt("-1.E+1"));
        ASSERT_EQUALS(false, MathLib::isInt("+1.E-1"));
        ASSERT_EQUALS(false, MathLib::isInt(" 1.0E+1"));
        // with whitespace in front
        ASSERT_EQUALS(false, MathLib::isInt(" 1.0E-1"));
        ASSERT_EQUALS(false, MathLib::isInt(" -1.0E+1"));
        ASSERT_EQUALS(false, MathLib::isInt(" +1.0E-1"));
        ASSERT_EQUALS(false, MathLib::isInt(" -1.E+1"));
        ASSERT_EQUALS(false, MathLib::isInt(" +1.E-1"));
        // with whitespace in front and end
        ASSERT_EQUALS(false, MathLib::isInt(" 1.0E-1  "));
        ASSERT_EQUALS(false, MathLib::isInt(" -1.0E+1  "));
        ASSERT_EQUALS(false, MathLib::isInt(" +1.0E-1  "));
        ASSERT_EQUALS(false, MathLib::isInt(" -1.E+1  "));
        ASSERT_EQUALS(false, MathLib::isInt(" +1.E-1  "));
        // with whitespace in front and end
        ASSERT_EQUALS(false, MathLib::isInt("1.0E-1  "));
        ASSERT_EQUALS(false, MathLib::isInt("-1.0E+1  "));
        ASSERT_EQUALS(false, MathLib::isInt("+1.0E-1  "));
        ASSERT_EQUALS(false, MathLib::isInt("-1.E+1  "));
        ASSERT_EQUALS(false, MathLib::isInt("+1.E-1  "));
        // test some garbage
        ASSERT_EQUALS(false, MathLib::isInt("u"));
        ASSERT_EQUALS(false, MathLib::isInt("l"));
        ASSERT_EQUALS(false, MathLib::isInt("ul"));
        ASSERT_EQUALS(false, MathLib::isInt("ll"));
        ASSERT_EQUALS(false, MathLib::isInt("U"));
        ASSERT_EQUALS(false, MathLib::isInt("L"));
        ASSERT_EQUALS(false, MathLib::isInt("uL"));
        ASSERT_EQUALS(false, MathLib::isInt("LL"));
        ASSERT_EQUALS(false, MathLib::isInt("e2"));
        ASSERT_EQUALS(false, MathLib::isInt("E2"));
        ASSERT_EQUALS(false, MathLib::isInt(".e2"));
        ASSERT_EQUALS(false, MathLib::isInt(".E2"));
        ASSERT_EQUALS(false, MathLib::isInt("0x"));
        ASSERT_EQUALS(false, MathLib::isInt("0xu"));
        ASSERT_EQUALS(false, MathLib::isInt("0xl"));
        ASSERT_EQUALS(false, MathLib::isInt("0xul"));
        // test empty string
        ASSERT_EQUALS(false, MathLib::isInt(""));
    }

    void isbin() const {
        // positive testing
        ASSERT_EQUALS(true, MathLib::isBin("0b0"));
        ASSERT_EQUALS(true, MathLib::isBin("0b1"));
        ASSERT_EQUALS(true, MathLib::isBin("+0b1"));
        ASSERT_EQUALS(true, MathLib::isBin("-0b1"));
        ASSERT_EQUALS(true, MathLib::isBin("0b11010111"));
        ASSERT_EQUALS(true, MathLib::isBin("-0b11010111"));
        ASSERT_EQUALS(true, MathLib::isBin("0B11010111"));
        ASSERT_EQUALS(true, MathLib::isBin("0b11010111u"));
        ASSERT_EQUALS(true, MathLib::isBin("0b11010111ul"));
        ASSERT_EQUALS(true, MathLib::isBin("0b11010111ull"));
        ASSERT_EQUALS(true, MathLib::isBin("0b11010111l"));
        ASSERT_EQUALS(true, MathLib::isBin("0b11010111ll"));
        ASSERT_EQUALS(true, MathLib::isBin("0b11010111llu"));
        ASSERT_EQUALS(true, MathLib::isBin("0b11010111l"));
        ASSERT_EQUALS(true, MathLib::isBin("0b11010111lu"));
        ASSERT_EQUALS(false, MathLib::isBin("0b11010111lul")); // Suffix LUL not allowed

        // negative testing
        ASSERT_EQUALS(false, MathLib::isBin("100101bx"));
        ASSERT_EQUALS(false, MathLib::isBin("0"));
        ASSERT_EQUALS(false, MathLib::isBin("0B"));
        ASSERT_EQUALS(false, MathLib::isBin("0C"));
        ASSERT_EQUALS(false, MathLib::isBin("+0B"));
        ASSERT_EQUALS(false, MathLib::isBin("-0B"));
        ASSERT_EQUALS(false, MathLib::isBin("-0Bx"));
        ASSERT_EQUALS(false, MathLib::isBin("0b11010111x"));
        ASSERT_EQUALS(false, MathLib::isBin("0b11010111ux"));
        ASSERT_EQUALS(false, MathLib::isBin("0b11010111lx"));
        ASSERT_EQUALS(false, MathLib::isBin("0b11010111lux"));
        ASSERT_EQUALS(false, MathLib::isBin("0b11010111ulx"));
        ASSERT_EQUALS(false, MathLib::isBin("0b11010111lulx"));
        ASSERT_EQUALS(false, MathLib::isBin("0b11010111ullx"));
        ASSERT_EQUALS(false, MathLib::isBin("0b11010111lll"));

        // test empty string
        ASSERT_EQUALS(false, MathLib::isBin(""));
    }

    void isnegative() const {
        ASSERT_EQUALS(true, MathLib::isNegative("-1"));
        ASSERT_EQUALS(true, MathLib::isNegative("-1."));
        ASSERT_EQUALS(true, MathLib::isNegative("-1.0"));
        ASSERT_EQUALS(true, MathLib::isNegative("-1.0E+2"));
        ASSERT_EQUALS(true, MathLib::isNegative("-1.0E-2"));

        ASSERT_EQUALS(false, MathLib::isNegative("+1"));
        ASSERT_EQUALS(false, MathLib::isNegative("+1."));
        ASSERT_EQUALS(false, MathLib::isNegative("+1.0"));
        ASSERT_EQUALS(false, MathLib::isNegative("+1.0E+2"));
        ASSERT_EQUALS(false, MathLib::isNegative("+1.0E-2"));
        // test empty string
        ASSERT_EQUALS(false, MathLib::isNegative(""));
    }

    void isoct() const {
        // octal number format: [+|-]0[0-7][suffix]
        // positive testing
        ASSERT_EQUALS(true, MathLib::isOct("010"));
        ASSERT_EQUALS(true, MathLib::isOct("+010"));
        ASSERT_EQUALS(true, MathLib::isOct("-010"));
        ASSERT_EQUALS(true, MathLib::isOct("0175"));
        ASSERT_EQUALS(true, MathLib::isOct("+0175"));
        ASSERT_EQUALS(true, MathLib::isOct("-0175"));
        ASSERT_EQUALS(true, MathLib::isOct("00"));
        ASSERT_EQUALS(true, MathLib::isOct("02"));
        ASSERT_EQUALS(true, MathLib::isOct("+042"));
        ASSERT_EQUALS(true, MathLib::isOct("-042"));
        ASSERT_EQUALS(true, MathLib::isOct("+042U"));
        ASSERT_EQUALS(true, MathLib::isOct("-042U"));
        ASSERT_EQUALS(true, MathLib::isOct("+042L"));
        ASSERT_EQUALS(true, MathLib::isOct("-042L"));
        ASSERT_EQUALS(true, MathLib::isOct("+042LU"));
        ASSERT_EQUALS(true, MathLib::isOct("-042LU"));
        ASSERT_EQUALS(true, MathLib::isOct("+042UL"));
        ASSERT_EQUALS(true, MathLib::isOct("-042UL"));
        ASSERT_EQUALS(true, MathLib::isOct("+042ULL"));
        ASSERT_EQUALS(true, MathLib::isOct("-042ULL"));
        ASSERT_EQUALS(true, MathLib::isOct("+042LLU"));
        ASSERT_EQUALS(true, MathLib::isOct("-042LLU"));

        // test empty string
        ASSERT_EQUALS(false, MathLib::isOct(""));

        // negative testing
        ASSERT_EQUALS(false, MathLib::isOct("0"));
        ASSERT_EQUALS(false, MathLib::isOct("-0x175"));
        ASSERT_EQUALS(false, MathLib::isOct("-0_garbage_"));
        ASSERT_EQUALS(false, MathLib::isOct("  "));
        ASSERT_EQUALS(false, MathLib::isOct(" "));
        ASSERT_EQUALS(false, MathLib::isOct("02."));
        ASSERT_EQUALS(false, MathLib::isOct("02E2"));
        ASSERT_EQUALS(false, MathLib::isOct("+042x"));
        ASSERT_EQUALS(false, MathLib::isOct("-042x"));
        ASSERT_EQUALS(false, MathLib::isOct("+042Ux"));
        ASSERT_EQUALS(false, MathLib::isOct("-042Ux"));
        ASSERT_EQUALS(false, MathLib::isOct("+042Lx"));
        ASSERT_EQUALS(false, MathLib::isOct("-042Lx"));
        ASSERT_EQUALS(false, MathLib::isOct("+042ULx"));
        ASSERT_EQUALS(false, MathLib::isOct("-042ULx"));
        ASSERT_EQUALS(false, MathLib::isOct("+042LLx"));
        ASSERT_EQUALS(false, MathLib::isOct("-042LLx"));
        ASSERT_EQUALS(false, MathLib::isOct("+042ULLx"));
        ASSERT_EQUALS(false, MathLib::isOct("-042ULLx"));
        ASSERT_EQUALS(false, MathLib::isOct("+042LLUx"));
        ASSERT_EQUALS(false, MathLib::isOct("-042LLUx"));
        ASSERT_EQUALS(false, MathLib::isOct("+042LUL"));
        ASSERT_EQUALS(false, MathLib::isOct("-042LUL"));
        // white space in front
        ASSERT_EQUALS(false, MathLib::isOct(" -042ULL"));
        // trailing white space
        ASSERT_EQUALS(false, MathLib::isOct("-042ULL  "));
        // front and trailing white space
        ASSERT_EQUALS(false, MathLib::isOct("  -042ULL  "));
        ASSERT_EQUALS(false, MathLib::isOct("+042LUL+0"));
    }

    void isFloatHex() const {
        // hex number syntax: [sign]0x[hexnumbers][suffix]
        ASSERT_EQUALS(true, MathLib::isFloatHex("0x1.999999999999ap-4"));
        ASSERT_EQUALS(true, MathLib::isFloatHex("0x0.3p10"));
        ASSERT_EQUALS(true, MathLib::isFloatHex("0x1.fp3"));
        ASSERT_EQUALS(true, MathLib::isFloatHex("0x1P-1"));
        ASSERT_EQUALS(true, MathLib::isFloatHex("0xcc.ccccccccccdp-11"));
        ASSERT_EQUALS(true, MathLib::isFloatHex("0x3.243F6A88p+03"));
        ASSERT_EQUALS(true, MathLib::isFloatHex("0xA.Fp-10"));
        ASSERT_EQUALS(true, MathLib::isFloatHex("0x1.2p-10f"));
        ASSERT_EQUALS(true, MathLib::isFloatHex("0x1.2p+10F"));
        ASSERT_EQUALS(true, MathLib::isFloatHex("0x1.2p+10l"));
        ASSERT_EQUALS(true, MathLib::isFloatHex("0x1.2p+10L"));
        ASSERT_EQUALS(true, MathLib::isFloatHex("0X.2p-0"));

        ASSERT_EQUALS(false, MathLib::isFloatHex(""));
        ASSERT_EQUALS(false, MathLib::isFloatHex("0"));
        ASSERT_EQUALS(false, MathLib::isFloatHex("0x"));
        ASSERT_EQUALS(false, MathLib::isFloatHex("0xa"));
        ASSERT_EQUALS(false, MathLib::isFloatHex("+0x"));
        ASSERT_EQUALS(false, MathLib::isFloatHex("-0x"));
        ASSERT_EQUALS(false, MathLib::isFloatHex("0x"));
        ASSERT_EQUALS(false, MathLib::isFloatHex("0x."));
        ASSERT_EQUALS(false, MathLib::isFloatHex("0XP"));
        ASSERT_EQUALS(false, MathLib::isFloatHex("0xx"));
        ASSERT_EQUALS(false, MathLib::isFloatHex("0x1P+-1"));
        ASSERT_EQUALS(false, MathLib::isFloatHex("0x1p+10e"));
        ASSERT_EQUALS(false, MathLib::isFloatHex("0x1p+1af"));
        ASSERT_EQUALS(false, MathLib::isFloatHex("0x1p+10LL"));
    }

    void isIntHex() const {
        // hex number syntax: [sign]0x[hexnumbers][suffix]

        // positive testing
        ASSERT_EQUALS(true, MathLib::isIntHex("0xa"));
        ASSERT_EQUALS(true, MathLib::isIntHex("0x2AF3"));
        ASSERT_EQUALS(true, MathLib::isIntHex("-0xa"));
        ASSERT_EQUALS(true, MathLib::isIntHex("-0x2AF3"));
        ASSERT_EQUALS(true, MathLib::isIntHex("+0xa"));
        ASSERT_EQUALS(true, MathLib::isIntHex("+0x2AF3"));
        ASSERT_EQUALS(true, MathLib::isIntHex("0x0"));
        ASSERT_EQUALS(true, MathLib::isIntHex("+0x0"));
        ASSERT_EQUALS(true, MathLib::isIntHex("-0x0"));
        ASSERT_EQUALS(true, MathLib::isIntHex("+0x0U"));
        ASSERT_EQUALS(true, MathLib::isIntHex("-0x0U"));
        ASSERT_EQUALS(true, MathLib::isIntHex("+0x0L"));
        ASSERT_EQUALS(true, MathLib::isIntHex("-0x0L"));
        ASSERT_EQUALS(true, MathLib::isIntHex("+0x0LU"));
        ASSERT_EQUALS(true, MathLib::isIntHex("-0x0LU"));
        ASSERT_EQUALS(true, MathLib::isIntHex("+0x0UL"));
        ASSERT_EQUALS(true, MathLib::isIntHex("-0x0UL"));
        ASSERT_EQUALS(true, MathLib::isIntHex("+0x0LL"));
        ASSERT_EQUALS(true, MathLib::isIntHex("-0x0LL"));
        ASSERT_EQUALS(true, MathLib::isIntHex("+0x0ULL"));
        ASSERT_EQUALS(true, MathLib::isIntHex("-0x0ULL"));
        ASSERT_EQUALS(true, MathLib::isIntHex("+0x0LLU"));
        ASSERT_EQUALS(true, MathLib::isIntHex("-0x0LLU"));
        ASSERT_EQUALS(true, MathLib::isIntHex("+0x0Z"));
        ASSERT_EQUALS(true, MathLib::isIntHex("-0x0Z"));
        ASSERT_EQUALS(true, MathLib::isIntHex("+0x0ZU"));
        ASSERT_EQUALS(true, MathLib::isIntHex("-0x0ZU"));
        ASSERT_EQUALS(true, MathLib::isIntHex("+0x0UZ"));
        ASSERT_EQUALS(true, MathLib::isIntHex("-0x0UZ"));
        ASSERT_EQUALS(true, MathLib::isIntHex("+0x0Uz"));
        ASSERT_EQUALS(true, MathLib::isIntHex("-0x0Uz"));

        // negative testing
        ASSERT_EQUALS(false, MathLib::isIntHex("+0x"));
        ASSERT_EQUALS(false, MathLib::isIntHex("-0x"));
        ASSERT_EQUALS(false, MathLib::isIntHex("0x"));
        ASSERT_EQUALS(false, MathLib::isIntHex("0xl"));
        ASSERT_EQUALS(false, MathLib::isIntHex("0xx"));
        ASSERT_EQUALS(false, MathLib::isIntHex("-0175"));
        ASSERT_EQUALS(false, MathLib::isIntHex("-0_garbage_"));
        ASSERT_EQUALS(false, MathLib::isIntHex("  "));
        ASSERT_EQUALS(false, MathLib::isIntHex(" "));
        ASSERT_EQUALS(false, MathLib::isIntHex("0"));
        ASSERT_EQUALS(false, MathLib::isIntHex("+0x0Lz"));
        ASSERT_EQUALS(false, MathLib::isIntHex("-0x0Lz"));
        ASSERT_EQUALS(false, MathLib::isIntHex("+0x0LUz"));
        ASSERT_EQUALS(false, MathLib::isIntHex("-0x0LUz"));
        ASSERT_EQUALS(false, MathLib::isIntHex("+0x0ULz"));
        ASSERT_EQUALS(false, MathLib::isIntHex("-0x0ULz"));
        ASSERT_EQUALS(false, MathLib::isIntHex("+0x0LLz"));
        ASSERT_EQUALS(false, MathLib::isIntHex("-0x0LLz"));
        ASSERT_EQUALS(false, MathLib::isIntHex("+0x0ULLz"));
        ASSERT_EQUALS(false, MathLib::isIntHex("-0x0ULLz"));
        ASSERT_EQUALS(false, MathLib::isIntHex("+0x0LLUz"));
        ASSERT_EQUALS(false, MathLib::isIntHex("-0x0LLUz"));
        ASSERT_EQUALS(false, MathLib::isIntHex("0x0+0"));
        ASSERT_EQUALS(false, MathLib::isIntHex("e2"));
        ASSERT_EQUALS(false, MathLib::isIntHex("+E2"));

        // test empty string
        ASSERT_EQUALS(false, MathLib::isIntHex(""));
    }

    void isValidIntegerSuffix() const {
        // negative testing
        ASSERT_EQUALS(false, MathLib::isValidIntegerSuffix(""));
        ASSERT_EQUALS(false, MathLib::isValidIntegerSuffix("ux"));
        ASSERT_EQUALS(false, MathLib::isValidIntegerSuffix("ulx"));
        ASSERT_EQUALS(false, MathLib::isValidIntegerSuffix("uu"));
        ASSERT_EQUALS(false, MathLib::isValidIntegerSuffix("lx"));
        ASSERT_EQUALS(false, MathLib::isValidIntegerSuffix("lux"));
        ASSERT_EQUALS(false, MathLib::isValidIntegerSuffix("lll"));
        ASSERT_EQUALS(false, MathLib::isValidIntegerSuffix("garbage"));
        ASSERT_EQUALS(false, MathLib::isValidIntegerSuffix("llu "));
        ASSERT_EQUALS(false, MathLib::isValidIntegerSuffix("i"));
        ASSERT_EQUALS(false, MathLib::isValidIntegerSuffix("iX"));
        ASSERT_EQUALS(false, MathLib::isValidIntegerSuffix("i6X"));
        ASSERT_EQUALS(false, MathLib::isValidIntegerSuffix("i64X"));
        ASSERT_EQUALS(false, MathLib::isValidIntegerSuffix("i64 "));
        ASSERT_EQUALS(false, MathLib::isValidIntegerSuffix("i66"));

        // positive testing
        ASSERT_EQUALS(true, MathLib::isValidIntegerSuffix("u"));
        ASSERT_EQUALS(true, MathLib::isValidIntegerSuffix("ul"));
        ASSERT_EQUALS(true, MathLib::isValidIntegerSuffix("ull"));
        ASSERT_EQUALS(true, MathLib::isValidIntegerSuffix("uz"));
        ASSERT_EQUALS(true, MathLib::isValidIntegerSuffix("l"));
        ASSERT_EQUALS(true, MathLib::isValidIntegerSuffix("lu"));
        ASSERT_EQUALS(true, MathLib::isValidIntegerSuffix("ll"));
        ASSERT_EQUALS(true, MathLib::isValidIntegerSuffix("llu"));
        ASSERT_EQUALS(true, MathLib::isValidIntegerSuffix("llU"));
        ASSERT_EQUALS(true, MathLib::isValidIntegerSuffix("LLU"));
        ASSERT_EQUALS(true, MathLib::isValidIntegerSuffix("z"));
        ASSERT_EQUALS(true, MathLib::isValidIntegerSuffix("Z"));
        ASSERT_EQUALS(true, MathLib::isValidIntegerSuffix("zu"));
        ASSERT_EQUALS(true, MathLib::isValidIntegerSuffix("UZ"));
        ASSERT_EQUALS(true, MathLib::isValidIntegerSuffix("ZU"));

        // Microsoft extensions:
        ASSERT_EQUALS(true, MathLib::isValidIntegerSuffix("i64"));
        ASSERT_EQUALS(true, MathLib::isValidIntegerSuffix("I64"));
        ASSERT_EQUALS(true, MathLib::isValidIntegerSuffix("ui64"));
        ASSERT_EQUALS(true, MathLib::isValidIntegerSuffix("UI64"));
        ASSERT_EQUALS(false, MathLib::isValidIntegerSuffix("i64", false));
        ASSERT_EQUALS(false, MathLib::isValidIntegerSuffix("I64", false));
        ASSERT_EQUALS(false, MathLib::isValidIntegerSuffix("ui64", false));
        ASSERT_EQUALS(false, MathLib::isValidIntegerSuffix("UI64", false));

        // User defined suffix literals
        ASSERT_EQUALS(false, MathLib::isValidIntegerSuffix("_"));
        ASSERT_EQUALS(true, MathLib::isValidIntegerSuffix("_MyUserDefinedLiteral"));
    }

    void ispositive() const {
        ASSERT_EQUALS(false, MathLib::isPositive("-1"));
        ASSERT_EQUALS(false, MathLib::isPositive("-1."));
        ASSERT_EQUALS(false, MathLib::isPositive("-1.0"));
        ASSERT_EQUALS(false, MathLib::isPositive("-1.0E+2"));
        ASSERT_EQUALS(false, MathLib::isPositive("-1.0E-2"));

        ASSERT_EQUALS(true, MathLib::isPositive("+1"));
        ASSERT_EQUALS(true, MathLib::isPositive("+1."));
        ASSERT_EQUALS(true, MathLib::isPositive("+1.0"));
        ASSERT_EQUALS(true, MathLib::isPositive("+1.0E+2"));
        ASSERT_EQUALS(true, MathLib::isPositive("+1.0E-2"));

        // test empty string
        ASSERT_EQUALS(false, MathLib::isPositive("")); // "" is neither positive nor negative
    }

    void isFloat() const {
        ASSERT_EQUALS(false, MathLib::isFloat(""));
        ASSERT_EQUALS(true,  MathLib::isFloat("0.f"));
        ASSERT_EQUALS(true,  MathLib::isFloat("0.f"));
        ASSERT_EQUALS(true,  MathLib::isFloat("0xA.Fp-10"));

        // User defined suffix literals
        ASSERT_EQUALS(false, MathLib::isFloat("0_"));
        ASSERT_EQUALS(false, MathLib::isFloat("0._"));
        ASSERT_EQUALS(false, MathLib::isFloat("0.1_"));
        ASSERT_EQUALS(true, MathLib::isFloat("0.0_MyUserDefinedLiteral"));
        ASSERT_EQUALS(true, MathLib::isFloat("0._MyUserDefinedLiteral"));
    }

    void isDecimalFloat() const {
        ASSERT_EQUALS(false, MathLib::isDecimalFloat(""));
        ASSERT_EQUALS(false, MathLib::isDecimalFloat("."));
        ASSERT_EQUALS(false, MathLib::isDecimalFloat("..."));
        ASSERT_EQUALS(false, MathLib::isDecimalFloat(".e"));
        ASSERT_EQUALS(false, MathLib::isDecimalFloat(".e2"));
        ASSERT_EQUALS(false, MathLib::isDecimalFloat(".E"));
        ASSERT_EQUALS(false, MathLib::isDecimalFloat("+E."));
        ASSERT_EQUALS(false, MathLib::isDecimalFloat("+e."));
        ASSERT_EQUALS(false, MathLib::isDecimalFloat("-E."));
        ASSERT_EQUALS(false, MathLib::isDecimalFloat("-e."));
        ASSERT_EQUALS(false, MathLib::isDecimalFloat("-X"));
        ASSERT_EQUALS(false, MathLib::isDecimalFloat("+X"));
        ASSERT_EQUALS(false, MathLib::isDecimalFloat("-."));
        ASSERT_EQUALS(false, MathLib::isDecimalFloat("-."));
        ASSERT_EQUALS(false, MathLib::isDecimalFloat("-"));
        ASSERT_EQUALS(false, MathLib::isDecimalFloat("+"));
        ASSERT_EQUALS(false, MathLib::isDecimalFloat(" "));

        ASSERT_EQUALS(false, MathLib::isDecimalFloat("0"));
        ASSERT_EQUALS(false, MathLib::isDecimalFloat("0 "));
        ASSERT_EQUALS(false, MathLib::isDecimalFloat(" 0 "));
        ASSERT_EQUALS(false, MathLib::isDecimalFloat(" 0"));

        ASSERT_EQUALS(true,  MathLib::isDecimalFloat("0."));
        ASSERT_EQUALS(true,  MathLib::isDecimalFloat("0.f"));
        ASSERT_EQUALS(true,  MathLib::isDecimalFloat("0.F"));
        ASSERT_EQUALS(true,  MathLib::isDecimalFloat("0.l"));
        ASSERT_EQUALS(true,  MathLib::isDecimalFloat("0.L"));
        ASSERT_EQUALS(false, MathLib::isDecimalFloat("0. "));
        ASSERT_EQUALS(false, MathLib::isDecimalFloat(" 0. "));
        ASSERT_EQUALS(false, MathLib::isDecimalFloat(" 0."));

        ASSERT_EQUALS(false, MathLib::isDecimalFloat("0.."));
        ASSERT_EQUALS(false, MathLib::isDecimalFloat("..0.."));
        ASSERT_EQUALS(false, MathLib::isDecimalFloat("..0"));
        ASSERT_EQUALS(true,  MathLib::isDecimalFloat("0.0"));
        ASSERT_EQUALS(true,  MathLib::isDecimalFloat("0.0f"));
        ASSERT_EQUALS(true,  MathLib::isDecimalFloat("0.0F"));
        ASSERT_EQUALS(true,  MathLib::isDecimalFloat("0.0l"));
        ASSERT_EQUALS(true,  MathLib::isDecimalFloat("0.0L"));
        ASSERT_EQUALS(true,  MathLib::isDecimalFloat("-0."));
        ASSERT_EQUALS(true,  MathLib::isDecimalFloat("+0."));
        ASSERT_EQUALS(true,  MathLib::isDecimalFloat("-0.0"));
        ASSERT_EQUALS(true,  MathLib::isDecimalFloat("+0.0"));
        ASSERT_EQUALS(true,  MathLib::isDecimalFloat("0E0"));
        ASSERT_EQUALS(true,  MathLib::isDecimalFloat("+0E0"));
        ASSERT_EQUALS(true,  MathLib::isDecimalFloat("+0E0"));
        ASSERT_EQUALS(true,  MathLib::isDecimalFloat("+0E+0"));
        ASSERT_EQUALS(true,  MathLib::isDecimalFloat("+0E-0"));
        ASSERT_EQUALS(true,  MathLib::isDecimalFloat("-0E+0"));
        ASSERT_EQUALS(true,  MathLib::isDecimalFloat("-0E-0"));
        ASSERT_EQUALS(true,  MathLib::isDecimalFloat("+0.0E+1"));
        ASSERT_EQUALS(true,  MathLib::isDecimalFloat("+0.0E-1"));
        ASSERT_EQUALS(true,  MathLib::isDecimalFloat("-0.0E+1"));
        ASSERT_EQUALS(true,  MathLib::isDecimalFloat("-0.0E-1"));

        ASSERT_EQUALS(false, MathLib::isDecimalFloat("1"));
        ASSERT_EQUALS(false, MathLib::isDecimalFloat("-1"));
        ASSERT_EQUALS(false, MathLib::isDecimalFloat("+1"));
        ASSERT_EQUALS(true,  MathLib::isDecimalFloat("+1e+1"));
        ASSERT_EQUALS(true,  MathLib::isDecimalFloat("+1E+1"));
        ASSERT_EQUALS(true,  MathLib::isDecimalFloat("+1E+100"));
        ASSERT_EQUALS(true,  MathLib::isDecimalFloat("+1E+100f"));
        ASSERT_EQUALS(true,  MathLib::isDecimalFloat("+1E+100F"));
        ASSERT_EQUALS(true,  MathLib::isDecimalFloat("+1E+100l"));
        ASSERT_EQUALS(true,  MathLib::isDecimalFloat("+1E+100L"));
        ASSERT_EQUALS(true,  MathLib::isDecimalFloat("+1E+007")); // to be sure about #5485
        ASSERT_EQUALS(true,  MathLib::isDecimalFloat("+1E+001f"));
        ASSERT_EQUALS(true,  MathLib::isDecimalFloat("+1E+001F"));
        ASSERT_EQUALS(true,  MathLib::isDecimalFloat("+1E+001l"));
        ASSERT_EQUALS(true,  MathLib::isDecimalFloat("+1E+001L"));
        ASSERT_EQUALS(true,  MathLib::isDecimalFloat("+1E+10000"));
        ASSERT_EQUALS(true,  MathLib::isDecimalFloat("-1E+1"));
        ASSERT_EQUALS(true,  MathLib::isDecimalFloat("-1E+10000"));
        ASSERT_EQUALS(true,  MathLib::isDecimalFloat(".1250E+04"));
        ASSERT_EQUALS(true,  MathLib::isDecimalFloat("-1E-1"));
        ASSERT_EQUALS(true,  MathLib::isDecimalFloat("-1E-10000"));
        ASSERT_EQUALS(true,  MathLib::isDecimalFloat("+1.23e+01"));
        ASSERT_EQUALS(true,  MathLib::isDecimalFloat("+1.23E+01"));
        ASSERT_EQUALS(false, MathLib::isDecimalFloat("+1e+x"));
        ASSERT_EQUALS(false, MathLib::isDecimalFloat("+1E+X"));
        ASSERT_EQUALS(false, MathLib::isDecimalFloat("+1E+001lX"));
        ASSERT_EQUALS(false, MathLib::isDecimalFloat("+1E+001LX"));
        ASSERT_EQUALS(false, MathLib::isDecimalFloat("+1E+001f2"));
        ASSERT_EQUALS(false, MathLib::isDecimalFloat("+1E+001F2"));
        ASSERT_EQUALS(false, MathLib::isDecimalFloat("+1e+003x"));
        ASSERT_EQUALS(false, MathLib::isDecimalFloat("+1E+003X"));


        ASSERT_EQUALS(true, MathLib::isDecimalFloat("0.4"));
        ASSERT_EQUALS(true, MathLib::isDecimalFloat("2352.3f"));
        ASSERT_EQUALS(true, MathLib::isDecimalFloat("2352.3F"));
        ASSERT_EQUALS(true, MathLib::isDecimalFloat("2352.3l"));
        ASSERT_EQUALS(true, MathLib::isDecimalFloat("2352.3L"));
        ASSERT_EQUALS(true, MathLib::isDecimalFloat("0.00004"));
        ASSERT_EQUALS(true, MathLib::isDecimalFloat("2352.00001f"));
        ASSERT_EQUALS(true, MathLib::isDecimalFloat("2352.00001F"));
        ASSERT_EQUALS(true, MathLib::isDecimalFloat("2352.00001l"));
        ASSERT_EQUALS(true, MathLib::isDecimalFloat("2352.00001L"));
        ASSERT_EQUALS(true, MathLib::isDecimalFloat(".4"));
        ASSERT_EQUALS(true, MathLib::isDecimalFloat(".3e2"));
        ASSERT_EQUALS(true, MathLib::isDecimalFloat("1.0E+1"));
        ASSERT_EQUALS(true, MathLib::isDecimalFloat("1.0E-1"));
        ASSERT_EQUALS(true, MathLib::isDecimalFloat("-1.0E+1"));

        // User defined suffix literals
        ASSERT_EQUALS(false, MathLib::isDecimalFloat("0_"));
        ASSERT_EQUALS(false, MathLib::isDecimalFloat(".1_"));
        ASSERT_EQUALS(false, MathLib::isDecimalFloat("0.1_"));
        ASSERT_EQUALS(true, MathLib::isDecimalFloat("0.0_MyUserDefinedLiteral"));
        ASSERT_EQUALS(true, MathLib::isDecimalFloat(".1_MyUserDefinedLiteral"));
    }

    void naninf() const {
        ASSERT_EQUALS("nan.0", MathLib::divide("0.0", "0.0")); // nan
        ASSERT_EQUALS("nan.0", MathLib::divide("0.0", "0.f")); // nan (#5875)
        ASSERT_EQUALS("nan.0", MathLib::divide("-0.0", "0.f")); // nan (#5875)
        ASSERT_EQUALS("nan.0", MathLib::divide("-0.f", "0.f")); // nan (#5875)
        ASSERT_EQUALS("nan.0", MathLib::divide("-0.0", "-0.f")); // nan (#5875)
        ASSERT_EQUALS("nan.0", MathLib::divide("-.0", "-0.f")); // nan (#5875)
        ASSERT_EQUALS("nan.0", MathLib::divide("0.0", "-0.f")); // nan (#5875)
        ASSERT_EQUALS("nan.0", MathLib::divide("0.f", "-0.f")); // nan (#5875)
        ASSERT_EQUALS("inf.0", MathLib::divide("3.0", "0.0")); // inf
        ASSERT_EQUALS("inf.0", MathLib::divide("3.0", "0.f")); // inf (#5875)
        ASSERT_EQUALS("-inf.0", MathLib::divide("-3.0", "0.0")); // -inf (#5142)
        ASSERT_EQUALS("-inf.0", MathLib::divide("-3.0", "0.0f")); // -inf (#5142)
        ASSERT_EQUALS("inf.0", MathLib::divide("-3.0", "-0.0f")); // inf (#5142)
    }

    void isdec() const {
        // positive testing
        ASSERT_EQUALS(true, MathLib::isDec("1"));
        ASSERT_EQUALS(true, MathLib::isDec("+1"));
        ASSERT_EQUALS(true, MathLib::isDec("-1"));
        ASSERT_EQUALS(true, MathLib::isDec("-100"));
        ASSERT_EQUALS(true, MathLib::isDec("-1L"));
        ASSERT_EQUALS(true, MathLib::isDec("1UL"));

        // negative testing
        ASSERT_EQUALS(false, MathLib::isDec("-1."));
        ASSERT_EQUALS(false, MathLib::isDec("+1."));
        ASSERT_EQUALS(false, MathLib::isDec("-x"));
        ASSERT_EQUALS(false, MathLib::isDec("+x"));
        ASSERT_EQUALS(false, MathLib::isDec("x"));
        ASSERT_EQUALS(false, MathLib::isDec(""));

        // User defined suffix literals
        ASSERT_EQUALS(false, MathLib::isDec("0_"));
        ASSERT_EQUALS(false, MathLib::isDec("+0_"));
        ASSERT_EQUALS(false, MathLib::isDec("-1_"));
        ASSERT_EQUALS(true, MathLib::isDec("0_MyUserDefinedLiteral"));
        ASSERT_EQUALS(true, MathLib::isDec("+1_MyUserDefinedLiteral"));
        ASSERT_EQUALS(true, MathLib::isDec("-1_MyUserDefinedLiteral"));
    }

    void isNullValue() const {
        // inter zero value
        ASSERT_EQUALS(true, MathLib::isNullValue("0"));
        ASSERT_EQUALS(true, MathLib::isNullValue("+0"));
        ASSERT_EQUALS(true, MathLib::isNullValue("-0"));
        // inter zero value (octal)
        ASSERT_EQUALS(true, MathLib::isNullValue("00"));
        ASSERT_EQUALS(true, MathLib::isNullValue("+00"));
        ASSERT_EQUALS(true, MathLib::isNullValue("-00"));
        // inter zero value (hex)
        ASSERT_EQUALS(true, MathLib::isNullValue("0x0"));
        ASSERT_EQUALS(true, MathLib::isNullValue("+0x0"));
        ASSERT_EQUALS(true, MathLib::isNullValue("-0x0"));
        ASSERT_EQUALS(true, MathLib::isNullValue("+0X0"));
        ASSERT_EQUALS(true, MathLib::isNullValue("-0X0"));
        // unsigned integer zero value
        ASSERT_EQUALS(true, MathLib::isNullValue("0U"));
        ASSERT_EQUALS(true, MathLib::isNullValue("+0U"));
        ASSERT_EQUALS(true, MathLib::isNullValue("-0U"));
        // long integer zero value
        ASSERT_EQUALS(true, MathLib::isNullValue("0L"));
        ASSERT_EQUALS(true, MathLib::isNullValue("+0L"));
        ASSERT_EQUALS(true, MathLib::isNullValue("-0L"));
        // unsigned long integer zero value
        ASSERT_EQUALS(true, MathLib::isNullValue("0UL"));
        ASSERT_EQUALS(true, MathLib::isNullValue("+0UL"));
        ASSERT_EQUALS(true, MathLib::isNullValue("-0UL"));
        // unsigned long integer zero value
        ASSERT_EQUALS(true, MathLib::isNullValue("0LU"));
        ASSERT_EQUALS(true, MathLib::isNullValue("+0LU"));
        ASSERT_EQUALS(true, MathLib::isNullValue("-0LU"));
        // long long integer zero value
        ASSERT_EQUALS(true, MathLib::isNullValue("0LL"));
        ASSERT_EQUALS(true, MathLib::isNullValue("+0LL"));
        ASSERT_EQUALS(true, MathLib::isNullValue("-0LL"));
        // long long unsigned zero value
        ASSERT_EQUALS(true, MathLib::isNullValue("0LLU"));
        ASSERT_EQUALS(true, MathLib::isNullValue("+0LLU"));
        ASSERT_EQUALS(true, MathLib::isNullValue("-0LLU"));
        // unsigned long long zero value
        ASSERT_EQUALS(true, MathLib::isNullValue("0ULL"));
        ASSERT_EQUALS(true, MathLib::isNullValue("+0ULL"));
        ASSERT_EQUALS(true, MathLib::isNullValue("-0ULL"));
        // floating pointer zero value (no trailing zero after dot)
        ASSERT_EQUALS(true, MathLib::isNullValue("0."));
        ASSERT_EQUALS(true, MathLib::isNullValue("+0."));
        ASSERT_EQUALS(true, MathLib::isNullValue("-0."));
        // floating pointer zero value (1 trailing zero after dot)
        ASSERT_EQUALS(true, MathLib::isNullValue("0.0"));
        ASSERT_EQUALS(true, MathLib::isNullValue("+0.0"));
        ASSERT_EQUALS(true, MathLib::isNullValue("-0.0"));
        // floating pointer zero value (3 trailing zeros after dot)
        ASSERT_EQUALS(true, MathLib::isNullValue("0.000"));
        ASSERT_EQUALS(true, MathLib::isNullValue("+0.000"));
        ASSERT_EQUALS(true, MathLib::isNullValue("-0.000"));
        // floating pointer zero value (no trailing zero after dot)
        ASSERT_EQUALS(true, MathLib::isNullValue("00."));
        ASSERT_EQUALS(true, MathLib::isNullValue("+00."));
        ASSERT_EQUALS(true, MathLib::isNullValue("-00."));
        // floating pointer zero value (1 trailing zero after dot)
        ASSERT_EQUALS(true, MathLib::isNullValue("00.0"));
        ASSERT_EQUALS(true, MathLib::isNullValue("+00.0"));
        ASSERT_EQUALS(true, MathLib::isNullValue("-00.0"));
        // floating pointer zero value (3 trailing zero after dot)
        ASSERT_EQUALS(true, MathLib::isNullValue("00.000"));
        ASSERT_EQUALS(true, MathLib::isNullValue("+00.000"));
        ASSERT_EQUALS(true, MathLib::isNullValue("-00.000"));
        // floating pointer zero value (3 trailing zero after dot)
        ASSERT_EQUALS(true, MathLib::isNullValue(".000"));
        // integer scientific notation
        ASSERT_EQUALS(true, MathLib::isNullValue("0E0"));
        ASSERT_EQUALS(true, MathLib::isNullValue("+0E0"));
        ASSERT_EQUALS(true, MathLib::isNullValue("-0E0"));
        ASSERT_EQUALS(true, MathLib::isNullValue("+0e0"));
        ASSERT_EQUALS(true, MathLib::isNullValue("-0e0"));
        // integer scientific notation
        ASSERT_EQUALS(true, MathLib::isNullValue("0E1"));
        ASSERT_EQUALS(true, MathLib::isNullValue("+0E1"));
        ASSERT_EQUALS(true, MathLib::isNullValue("-0E1"));
        // integer scientific notation
        ASSERT_EQUALS(true, MathLib::isNullValue("0E42"));
        ASSERT_EQUALS(true, MathLib::isNullValue("+0E42"));
        ASSERT_EQUALS(true, MathLib::isNullValue("-0E42"));
        // integer scientific notation
        ASSERT_EQUALS(true, MathLib::isNullValue("0E429999"));
        ASSERT_EQUALS(true, MathLib::isNullValue("+0E429999"));
        ASSERT_EQUALS(true, MathLib::isNullValue("-0E429999"));
        // integer scientific notation
        ASSERT_EQUALS(true, MathLib::isNullValue("0E+1"));
        ASSERT_EQUALS(true, MathLib::isNullValue("+0E+1"));
        ASSERT_EQUALS(true, MathLib::isNullValue("-0E+1"));
        // integer scientific notation
        ASSERT_EQUALS(true, MathLib::isNullValue("0E+42"));
        ASSERT_EQUALS(true, MathLib::isNullValue("+0E+42"));
        ASSERT_EQUALS(true, MathLib::isNullValue("-0E+42"));
        // integer scientific notation
        ASSERT_EQUALS(true, MathLib::isNullValue("0E+429999"));
        ASSERT_EQUALS(true, MathLib::isNullValue("+0E+429999"));
        ASSERT_EQUALS(true, MathLib::isNullValue("-0E+429999"));
        // integer scientific notation
        ASSERT_EQUALS(true, MathLib::isNullValue("0E-1"));
        ASSERT_EQUALS(true, MathLib::isNullValue("+0E-1"));
        ASSERT_EQUALS(true, MathLib::isNullValue("-0E-1"));
        // integer scientific notation
        ASSERT_EQUALS(true, MathLib::isNullValue("0E-42"));
        ASSERT_EQUALS(true, MathLib::isNullValue("+0E-42"));
        ASSERT_EQUALS(true, MathLib::isNullValue("-0E-42"));
        // integer scientific notation
        ASSERT_EQUALS(true, MathLib::isNullValue("0E-429999"));
        ASSERT_EQUALS(true, MathLib::isNullValue("+0E-429999"));
        ASSERT_EQUALS(true, MathLib::isNullValue("-0E-429999"));
        // floating point scientific notation
        ASSERT_EQUALS(true, MathLib::isNullValue("0.E0"));
        ASSERT_EQUALS(true, MathLib::isNullValue("+0.E0"));
        ASSERT_EQUALS(true, MathLib::isNullValue("-0.E0"));
        // floating point scientific notation
        ASSERT_EQUALS(true, MathLib::isNullValue("0.E-0"));
        ASSERT_EQUALS(true, MathLib::isNullValue("+0.E-0"));
        ASSERT_EQUALS(true, MathLib::isNullValue("-0.E+0"));
        // floating point scientific notation
        ASSERT_EQUALS(true, MathLib::isNullValue("0.E+0"));
        ASSERT_EQUALS(true, MathLib::isNullValue("+0.E+0"));
        ASSERT_EQUALS(true, MathLib::isNullValue("-0.E+0"));
        // floating point scientific notation
        ASSERT_EQUALS(true, MathLib::isNullValue("0.00E-0"));
        ASSERT_EQUALS(true, MathLib::isNullValue("+0.00E-0"));
        ASSERT_EQUALS(true, MathLib::isNullValue("-0.00E-0"));
        // floating point scientific notation
        ASSERT_EQUALS(true, MathLib::isNullValue("00000.00E-000000000"));
        ASSERT_EQUALS(true, MathLib::isNullValue("+00000.00E-000000000"));
        ASSERT_EQUALS(true, MathLib::isNullValue("-00000.00E-000000000"));
        // floating point scientific notation (suffix f)
        ASSERT_EQUALS(true, MathLib::isNullValue("0.f"));
        ASSERT_EQUALS(true, MathLib::isNullValue("+0.f"));
        ASSERT_EQUALS(true, MathLib::isNullValue("-0.f"));
        // floating point scientific notation (suffix f)
        ASSERT_EQUALS(true, MathLib::isNullValue("0.0f"));
        ASSERT_EQUALS(true, MathLib::isNullValue("0.0F"));
        ASSERT_EQUALS(true, MathLib::isNullValue("+0.0f"));
        ASSERT_EQUALS(true, MathLib::isNullValue("-0.0f"));
        // floating point scientific notation (suffix f)
        ASSERT_EQUALS(true, MathLib::isNullValue("00.0f"));
        ASSERT_EQUALS(true, MathLib::isNullValue("+00.0f"));
        ASSERT_EQUALS(true, MathLib::isNullValue("-00.0f"));
        // floating point scientific notation (suffix f)
        ASSERT_EQUALS(true, MathLib::isNullValue("00.00f"));
        ASSERT_EQUALS(true, MathLib::isNullValue("+00.00f"));
        ASSERT_EQUALS(true, MathLib::isNullValue("-00.00f"));
        // floating point scientific notation (suffix f)
        ASSERT_EQUALS(true, MathLib::isNullValue("00.00E+1f"));
        ASSERT_EQUALS(true, MathLib::isNullValue("+00.00E+1f"));
        ASSERT_EQUALS(true, MathLib::isNullValue("-00.00E+1f"));
        // hex float
        ASSERT_EQUALS(true, MathLib::isNullValue("0x0p3"));
        ASSERT_EQUALS(true, MathLib::isNullValue("0X0P3"));
        ASSERT_EQUALS(true, MathLib::isNullValue("0X0p-3"));
        ASSERT_EQUALS(true, MathLib::isNullValue("-0x0p3"));
        ASSERT_EQUALS(true, MathLib::isNullValue("+0x0p3"));

        // binary numbers
        ASSERT_EQUALS(true, MathLib::isNullValue("0b00"));
        ASSERT_EQUALS(true, MathLib::isNullValue("+0b00"));
        ASSERT_EQUALS(true, MathLib::isNullValue("-0b00"));
        // binary numbers (long)
        ASSERT_EQUALS(true, MathLib::isNullValue("0b00L"));
        ASSERT_EQUALS(true, MathLib::isNullValue("+0b00L"));
        ASSERT_EQUALS(true, MathLib::isNullValue("-0b00L"));
        // negative testing
        ASSERT_EQUALS(false, MathLib::isNullValue("0.1"));
        ASSERT_EQUALS(false, MathLib::isNullValue("1.0"));
        ASSERT_EQUALS(false, MathLib::isNullValue("0.01"));
        ASSERT_EQUALS(false, MathLib::isNullValue("0xF"));
        ASSERT_EQUALS(false, MathLib::isNullValue("0XFF"));
        ASSERT_EQUALS(false, MathLib::isNullValue("0b01"));
        ASSERT_EQUALS(false, MathLib::isNullValue("0B01"));
        ASSERT_EQUALS(false, MathLib::isNullValue("0x1p0"));
        ASSERT_EQUALS(false, MathLib::isNullValue("0x1P0"));
        ASSERT_EQUALS(false, MathLib::isNullValue("0xap0"));
        ASSERT_EQUALS(false, MathLib::isNullValue("0xbp0"));
        ASSERT_EQUALS(false, MathLib::isNullValue("0xcp0"));
        ASSERT_EQUALS(false, MathLib::isNullValue("0xdp0"));
        ASSERT_EQUALS(false, MathLib::isNullValue("0xep0"));
        ASSERT_EQUALS(false, MathLib::isNullValue("0xfp0"));
        ASSERT_EQUALS(false, MathLib::isNullValue("-00.01e-12"));
        ASSERT_EQUALS(false, MathLib::isNullValue("-00.01e+12"));
        ASSERT_EQUALS(false, MathLib::isNullValue(""));
        ASSERT_EQUALS(false, MathLib::isNullValue(" "));
        ASSERT_EQUALS(false, MathLib::isNullValue("x"));
        ASSERT_EQUALS(false, MathLib::isNullValue("garbage"));
        ASSERT_EQUALS(false, MathLib::isNullValue("UL"));
        ASSERT_EQUALS(false, MathLib::isNullValue("-ENOMEM"));
    }

    void sin() const {
        ASSERT_EQUALS("0.0", MathLib::sin("0"));
    }
    void cos() const {
        ASSERT_EQUALS("1.0", MathLib::cos("0"));
    }
    void tan() const {
        ASSERT_EQUALS("0.0", MathLib::tan("0"));
    }
    void abs() const {
        ASSERT_EQUALS("", MathLib::abs(""));
        ASSERT_EQUALS("0", MathLib::abs("0"));
        ASSERT_EQUALS("+0", MathLib::abs("+0"));
        ASSERT_EQUALS("0", MathLib::abs("-0"));
        ASSERT_EQUALS("+1", MathLib::abs("+1"));
        ASSERT_EQUALS("+1.0", MathLib::abs("+1.0"));
        ASSERT_EQUALS("1", MathLib::abs("-1"));
        ASSERT_EQUALS("1.0", MathLib::abs("-1.0"));
        ASSERT_EQUALS("9007199254740991", MathLib::abs("9007199254740991"));
    }

    void toString() const {
        ASSERT_EQUALS("0.0", MathLib::toString(0.0));
        ASSERT_EQUALS("0.0", MathLib::toString(+0.0));
        ASSERT_EQUALS("0.0", MathLib::toString(-0.0));

        ASSERT_EQUALS("1e-08", MathLib::toString(0.00000001));
        ASSERT_EQUALS("-1e-08", MathLib::toString(-0.00000001));

        ASSERT_EQUALS("2.22507385851e-308", MathLib::toString(std::numeric_limits<double>::min()));
        ASSERT_EQUALS("1.79769313486e+308", MathLib::toString(std::numeric_limits<double>::max()));
    }

    void CPP14DigitSeparators() const { // Ticket #7137, #7565
        ASSERT(MathLib::isDigitSeparator("'", 0) == false);
        ASSERT(MathLib::isDigitSeparator("123'0;", 3));
        ASSERT(MathLib::isDigitSeparator("foo(1'2);", 5));
        ASSERT(MathLib::isDigitSeparator("foo(1,1'2);", 7));
        ASSERT(MathLib::isDigitSeparator("int a=1'234-1'2-'0';", 7));
        ASSERT(MathLib::isDigitSeparator("int a=1'234-1'2-'0';", 13));
        ASSERT(MathLib::isDigitSeparator("int a=1'234-1'2-'0';", 16) == false);
        ASSERT(MathLib::isDigitSeparator("int b=1+9'8;", 9));
        ASSERT(MathLib::isDigitSeparator("if (1'2) { char c = 'c'; }", 5));
        ASSERT(MathLib::isDigitSeparator("if (120%1'2) { char c = 'c'; }", 9));
        ASSERT(MathLib::isDigitSeparator("if (120&1'2) { char c = 'c'; }", 9));
        ASSERT(MathLib::isDigitSeparator("if (120|1'2) { char c = 'c'; }", 9));
        ASSERT(MathLib::isDigitSeparator("if (120%1'2) { char c = 'c'; }", 24) == false);
        ASSERT(MathLib::isDigitSeparator("if (120%1'2) { char c = 'c'; }", 26) == false);
        ASSERT(MathLib::isDigitSeparator("0b0000001'0010'01110", 14));
    }
};

REGISTER_TEST(TestMathLib)
