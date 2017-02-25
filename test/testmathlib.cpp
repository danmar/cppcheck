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

#include "mathlib.h"
#include "testsuite.h"


class TestMathLib : public TestFixture {
public:
    TestMathLib() : TestFixture("TestMathLib") {
    }

private:

    void run() {
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
        TEST_CASE(isGreater)
        TEST_CASE(isGreaterEqual)
        TEST_CASE(isEqual)
        TEST_CASE(isNotEqual)
        TEST_CASE(isLess)
        TEST_CASE(isLessEqual)
        TEST_CASE(calculate);
        TEST_CASE(calculate1);
        TEST_CASE(typesuffix);
        TEST_CASE(toLongNumber);
        TEST_CASE(toDoubleNumber);
        TEST_CASE(naninf);
        TEST_CASE(isNullValue);
        TEST_CASE(incdec);
        TEST_CASE(sin);
        TEST_CASE(cos);
        TEST_CASE(tan);
        TEST_CASE(abs);
        TEST_CASE(toString);
        TEST_CASE(characterLiteralsNormalization);
        TEST_CASE(CPP14DigitSeparators);
    }

    void isGreater() const {
        ASSERT_EQUALS(true , MathLib::isGreater("1.0", "0.001"));
        ASSERT_EQUALS(false, MathLib::isGreater("-1.0", "0.001"));
    }

    void isGreaterEqual() const {
        ASSERT_EQUALS(true , MathLib::isGreaterEqual("1.00", "1.0"));
        ASSERT_EQUALS(true , MathLib::isGreaterEqual("1.001", "1.0"));
        ASSERT_EQUALS(true , MathLib::isGreaterEqual("1.0", "0.001"));
        ASSERT_EQUALS(false, MathLib::isGreaterEqual("-1.0", "0.001"));
    }

    void isEqual() const {
        ASSERT_EQUALS(true  , MathLib::isEqual("1.0", "1.0"));
        ASSERT_EQUALS(false , MathLib::isEqual("1.", "1.01"));
        ASSERT_EQUALS(true  , MathLib::isEqual("0.1","1.0E-1"));
    }

    void isNotEqual() const {
        ASSERT_EQUALS(false , MathLib::isNotEqual("1.0", "1.0"));
        ASSERT_EQUALS(true  , MathLib::isNotEqual("1.", "1.01"));
    }

    void isLess() const {
        ASSERT_EQUALS(false , MathLib::isLess("1.0", "0.001"));
        ASSERT_EQUALS(true  , MathLib::isLess("-1.0", "0.001"));
    }

    void isLessEqual() const {
        ASSERT_EQUALS(true  , MathLib::isLessEqual("1.00", "1.0"));
        ASSERT_EQUALS(false , MathLib::isLessEqual("1.001", "1.0"));
        ASSERT_EQUALS(false , MathLib::isLessEqual("1.0", "0.001"));
        ASSERT_EQUALS(true  , MathLib::isLessEqual("-1.0", "0.001"));
    }

    void calculate() const {
        // addition
        ASSERT_EQUALS("256", MathLib::add("0xff", "1"));
        ASSERT_EQUALS("249", MathLib::add("250", "-1"));
        ASSERT_EQUALS("251", MathLib::add("250", "1"));
        ASSERT_EQUALS("-2.0" , MathLib::add("-1.", "-1"));
        ASSERT_EQUALS("-1" , MathLib::add("0", "-1"));
        ASSERT_EQUALS("1"  , MathLib::add("1", "0"));
        ASSERT_EQUALS("0.0"  , MathLib::add("0", "0."));
        ASSERT_EQUALS("1.00000001"  , MathLib::add("1", "0.00000001"));  // #4016
        ASSERT_EQUALS("30666.22"  , MathLib::add("30666.22", "0.0"));  // #4068

        // subtraction
        ASSERT_EQUALS("254", MathLib::subtract("0xff", "1"));
        ASSERT_EQUALS("251", MathLib::subtract("250", "-1"));
        ASSERT_EQUALS("249", MathLib::subtract("250", "1"));
        ASSERT_EQUALS("0.0"  , MathLib::subtract("-1.", "-1"));
        ASSERT_EQUALS("1"  , MathLib::subtract("0", "-1"));
        ASSERT_EQUALS("1"  , MathLib::subtract("1", "0"));
        ASSERT_EQUALS("0.0"  , MathLib::subtract("0", "0."));
        ASSERT_EQUALS("0.99999999"  , MathLib::subtract("1", "0.00000001")); // #4016
        ASSERT_EQUALS("30666.22"  , MathLib::subtract("30666.22", "0.0"));  // #4068
        ASSERT_EQUALS("0.0" , MathLib::subtract("0.0", "0.0"));

        // multiply
        ASSERT_EQUALS("-0.003"          , MathLib::multiply("-1e-3", "3"));
        ASSERT_EQUALS("-11.96"          , MathLib::multiply("-2.3", "5.2"));
        ASSERT_EQUALS("3000.0"          , MathLib::multiply("1E3", "3"));
        ASSERT_EQUALS("3000.0"          , MathLib::multiply("1E+3", "3"));
        ASSERT_EQUALS("3000.0"          , MathLib::multiply("1.0E3", "3"));
        ASSERT_EQUALS("-3000.0"         , MathLib::multiply("-1.0E3", "3"));
        ASSERT_EQUALS("-3000.0"         , MathLib::multiply("-1.0E+3", "3"));
        ASSERT_EQUALS("0.0"             , MathLib::multiply("+1.0E+3", "0"));
        ASSERT_EQUALS("2147483648"      , MathLib::multiply("2","1073741824"));
        ASSERT_EQUALS("536870912"       , MathLib::multiply("512","1048576"));

        // divide
        ASSERT_EQUALS("1"   , MathLib::divide("1", "1"));
        ASSERT_EQUALS("0"   , MathLib::divide("0", "1"));
        ASSERT_EQUALS("5"   , MathLib::divide("-10", "-2"));
        ASSERT_EQUALS("-2.5", MathLib::divide("-10.", "4"));
        ASSERT_EQUALS("2.5" , MathLib::divide("-10.", "-4"));
        ASSERT_EQUALS("5.0" , MathLib::divide("25.5", "5.1"));
        ASSERT_EQUALS("7.0" , MathLib::divide("21.", "3"));
        ASSERT_EQUALS("1"   , MathLib::divide("3", "2"));
        ASSERT_THROW(MathLib::divide("123", "0"), InternalError); // throw
        ASSERT_THROW(MathLib::divide("-9223372036854775808", "-1"), InternalError); // #4520 - out of range => throw
        ASSERT_EQUALS("4611686018427387904",  MathLib::divide("-9223372036854775808", "-2")); // #6679
        MathLib::divide("123", "0.0"); // don't throw

        // Unknown action should throw exception
        ASSERT_THROW(MathLib::calculate("1","2",'j'),InternalError);
    }

    void calculate1() const {
        ASSERT_EQUALS("0"    , MathLib::calculate("2"    , "1"    , '%'));
        ASSERT_EQUALS("2"    , MathLib::calculate("12"   , "5"   , '%'));
        ASSERT_EQUALS("1"    , MathLib::calculate("100"  , "3"   , '%'));

#ifndef TEST_MATHLIB_VALUE
        // floating point modulo is not defined in C/C++
        ASSERT_EQUALS("0.0"  , MathLib::calculate("2.0"  , "1.0"  , '%'));
        ASSERT_EQUALS("12.0" , MathLib::calculate("12.0" , "13.0" , '%'));
        ASSERT_EQUALS("1.3"  , MathLib::calculate("5.3"  , "2.0" , '%'));
        ASSERT_EQUALS("1.7"  , MathLib::calculate("18.5" , "4.2" , '%'));
        MathLib::calculate("123", "0.0", '%'); // don't throw
#endif

        ASSERT_THROW(MathLib::calculate("123", "0", '%'), InternalError); // throw

        ASSERT_EQUALS("0"    , MathLib::calculate("1"    , "1"    , '^'));
        ASSERT_EQUALS("3"    , MathLib::calculate("2"    , "1"    , '^'));
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
        // from hex
        ASSERT_EQUALS(0     , MathLib::toLongNumber("0x0"));
        ASSERT_EQUALS(0     , MathLib::toLongNumber("-0x0"));
        ASSERT_EQUALS(0     , MathLib::toLongNumber("+0x0"));
        ASSERT_EQUALS(10    , MathLib::toLongNumber("0xa"));
        ASSERT_EQUALS(10995 , MathLib::toLongNumber("0x2AF3"));
        ASSERT_EQUALS(-10   , MathLib::toLongNumber("-0xa"));
        ASSERT_EQUALS(-10995, MathLib::toLongNumber("-0x2AF3"));
        ASSERT_EQUALS(10    , MathLib::toLongNumber("+0xa"));
        ASSERT_EQUALS(10995 , MathLib::toLongNumber("+0x2AF3"));

        // from octal
        ASSERT_EQUALS(8     , MathLib::toLongNumber("010"));
        ASSERT_EQUALS(8     , MathLib::toLongNumber("+010"));
        ASSERT_EQUALS(-8    , MathLib::toLongNumber("-010"));
        ASSERT_EQUALS(125   , MathLib::toLongNumber("0175"));
        ASSERT_EQUALS(125   , MathLib::toLongNumber("+0175"));
        ASSERT_EQUALS(-125  , MathLib::toLongNumber("-0175"));

        // from binary
        ASSERT_EQUALS(0     , MathLib::toLongNumber("0b0"));
        ASSERT_EQUALS(1     , MathLib::toLongNumber("0b1"));
        ASSERT_EQUALS(1     , MathLib::toLongNumber("+0b1"));
        ASSERT_EQUALS(-1    , MathLib::toLongNumber("-0b1"));
        ASSERT_EQUALS(215   , MathLib::toLongNumber("0b11010111"));
        ASSERT_EQUALS(-215  , MathLib::toLongNumber("-0b11010111"));
        ASSERT_EQUALS(215   , MathLib::toLongNumber("0B11010111"));

        // from base 10
        ASSERT_EQUALS(10    , MathLib::toLongNumber("10"));
        ASSERT_EQUALS(10    , MathLib::toLongNumber("10."));
        ASSERT_EQUALS(10    , MathLib::toLongNumber("10.0"));
        ASSERT_EQUALS(100   , MathLib::toLongNumber("10E+1"));
        ASSERT_EQUALS(1     , MathLib::toLongNumber("10E-1"));
        ASSERT_EQUALS(100   , MathLib::toLongNumber("+10E+1"));
        ASSERT_EQUALS(-1    , MathLib::toLongNumber("-10E-1"));
        ASSERT_EQUALS(100   , MathLib::toLongNumber("+10.E+1"));
        ASSERT_EQUALS(-1    , MathLib::toLongNumber("-10.E-1"));
        ASSERT_EQUALS(100   , MathLib::toLongNumber("+10.0E+1"));
        ASSERT_EQUALS(-1    , MathLib::toLongNumber("-10.0E-1"));

        // from char
        ASSERT_EQUALS((int)('A'),    MathLib::toLongNumber("'A'"));
        ASSERT_EQUALS((int)('\x10'), MathLib::toLongNumber("'\\x10'"));
        ASSERT_EQUALS((int)('\100'), MathLib::toLongNumber("'\\100'"));
        ASSERT_EQUALS((int)('\200'), MathLib::toLongNumber("'\\200'"));
#ifdef __GNUC__
        // BEGIN Implementation-specific results
        ASSERT_EQUALS((int)('AB'),    MathLib::toLongNumber("'AB'"));
        ASSERT_EQUALS((int)('ABC'),    MathLib::toLongNumber("'ABC'"));
        ASSERT_EQUALS((int)('ABCD'),    MathLib::toLongNumber("'ABCD'"));
        ASSERT_EQUALS((int)('ABCDE'),    MathLib::toLongNumber("'ABCDE'"));
        // END Implementation-specific results
#endif
        ASSERT_EQUALS((int)('\0'),   MathLib::toLongNumber("'\\0'"));
        ASSERT_EQUALS(0x1B,   MathLib::toLongNumber("'\\e'"));
        ASSERT_EQUALS((int)('\r'),   MathLib::toLongNumber("'\\r'"));
        ASSERT_EQUALS((int)('\x12'), MathLib::toLongNumber("'\\x12'"));
        // may cause some compile problems: ASSERT_EQUALS((int)('\x123'), MathLib::toLongNumber("'\\x123'"));
        // may cause some compile problems: ASSERT_EQUALS((int)('\x1234'), MathLib::toLongNumber("'\\x1234'"));
        ASSERT_EQUALS((int)('\3'),  MathLib::toLongNumber("'\\3'"));
        ASSERT_EQUALS((int)('\34'),  MathLib::toLongNumber("'\\34'"));
        ASSERT_EQUALS((int)('\034'), MathLib::toLongNumber("'\\034'"));
        ASSERT_EQUALS((int)('\x34'), MathLib::toLongNumber("'\\x34'"));
        ASSERT_EQUALS((int)('\134'), MathLib::toLongNumber("'\\134'"));
        ASSERT_EQUALS((int)('\134t'), MathLib::toLongNumber("'\\134t'")); // Ticket #7452
        ASSERT_THROW(MathLib::toLongNumber("'\\9'"), InternalError);
        ASSERT_THROW(MathLib::toLongNumber("'\\934'"), InternalError);
        // that is not gcc/clang encoding
        ASSERT_EQUALS(959657011, MathLib::toLongNumber("'\\u9343'"));
        ASSERT_EQUALS(1714631779, MathLib::toLongNumber("'\\U0001f34c'"));
        {
            // some unit-testing for a utility function
            ASSERT_EQUALS(0, MathLib::characterLiteralToLongNumber(std::string("")));
            ASSERT_EQUALS(32, MathLib::characterLiteralToLongNumber(std::string(" ")));
            ASSERT_EQUALS(538976288, MathLib::characterLiteralToLongNumber(std::string("          ")));
            ASSERT_THROW(MathLib::characterLiteralToLongNumber(std::string("\\u")), InternalError);
        }

        ASSERT_EQUALS(-8552249625308161526, MathLib::toLongNumber("0x89504e470d0a1a0a"));
        ASSERT_EQUALS(-8481036456200365558, MathLib::toLongNumber("0x8a4d4e470d0a1a0a"));
        ASSERT_EQUALS(9894494448401390090ULL, MathLib::toULongNumber("0x89504e470d0a1a0a"));
        ASSERT_EQUALS(9965707617509186058ULL, MathLib::toULongNumber("0x8a4d4e470d0a1a0a"));

        // zero input
        ASSERT_EQUALS(0    , MathLib::toULongNumber("0"));
        ASSERT_EQUALS(0    , MathLib::toULongNumber("-0"));
        ASSERT_EQUALS(0    , MathLib::toULongNumber("+0"));
        ASSERT_EQUALS(0U   , MathLib::toULongNumber("0U"));
        ASSERT_EQUALS(0    , MathLib::toULongNumber("-0x0"));

        ASSERT_EQUALS(1U    , MathLib::toULongNumber("1U"));
        ASSERT_EQUALS(10000U    , MathLib::toULongNumber("1e4"));
        ASSERT_EQUALS(10000U    , MathLib::toULongNumber("1e4"));
        ASSERT_EQUALS(0xFF00000000000000UL, MathLib::toULongNumber("0xFF00000000000000UL"));
        ASSERT_EQUALS(0x0A00000000000000UL, MathLib::toULongNumber("0x0A00000000000000UL"));
        ASSERT_EQUALS(9U, MathLib::toULongNumber("011"));
        ASSERT_EQUALS(5U, MathLib::toULongNumber("0b101"));

        // from long long
        /*
         * ASSERT_EQUALS(0xFF00000000000000LL, MathLib::toLongNumber("0xFF00000000000000LL"));
         * This does not work in a portable way!
         * While it succeeds on 32bit Visual Studio it fails on Linux 64bit because it is greater than 0x7FFFFFFFFFFFFFFF (=LLONG_MAX)
         */

        ASSERT_EQUALS(0x0A00000000000000LL, MathLib::toLongNumber("0x0A00000000000000LL"));
    }

    void toDoubleNumber() const {
        ASSERT_EQUALS_DOUBLE(10.0  , MathLib::toDoubleNumber("10"));
        ASSERT_EQUALS_DOUBLE(1000.0, MathLib::toDoubleNumber("10E+2"));
        ASSERT_EQUALS_DOUBLE(100.0 , MathLib::toDoubleNumber("1.0E+2"));
        ASSERT_EQUALS_DOUBLE(-100.0, MathLib::toDoubleNumber("-1.0E+2"));
        ASSERT_EQUALS_DOUBLE(-1e+10, MathLib::toDoubleNumber("-1.0E+10"));
        ASSERT_EQUALS_DOUBLE(100.0 , MathLib::toDoubleNumber("+1.0E+2"));
        ASSERT_EQUALS_DOUBLE(1e+10 , MathLib::toDoubleNumber("+1.0E+10"));
        ASSERT_EQUALS_DOUBLE(100.0 , MathLib::toDoubleNumber("1.0E+2"));
        ASSERT_EQUALS_DOUBLE(1e+10 , MathLib::toDoubleNumber("1.0E+10"));
        ASSERT_EQUALS_DOUBLE(0.0   , MathLib::toDoubleNumber("0E+0"));
        ASSERT_EQUALS_DOUBLE(0.0   , MathLib::toDoubleNumber("0E-0"));
        ASSERT_EQUALS_DOUBLE(0.0   , MathLib::toDoubleNumber("0E+00"));
        ASSERT_EQUALS_DOUBLE(0.0   , MathLib::toDoubleNumber("0E-00"));
        ASSERT_EQUALS_DOUBLE(0.0   , MathLib::toDoubleNumber("-0E+00"));
        ASSERT_EQUALS_DOUBLE(0.0   , MathLib::toDoubleNumber("+0E-00"));
        ASSERT_EQUALS_DOUBLE(0.0   , MathLib::toDoubleNumber("0"));
        ASSERT_EQUALS_DOUBLE(0.0   , MathLib::toDoubleNumber("0."));
        ASSERT_EQUALS_DOUBLE(0.0   , MathLib::toDoubleNumber("0.0"));
        ASSERT_EQUALS_DOUBLE(0.0   , MathLib::toDoubleNumber("-0"));
        ASSERT_EQUALS_DOUBLE(0.0   , MathLib::toDoubleNumber("+0"));
        ASSERT_EQUALS_DOUBLE(0.0   , MathLib::toDoubleNumber("-0."));
        ASSERT_EQUALS_DOUBLE(0.0   , MathLib::toDoubleNumber("+0."));
        ASSERT_EQUALS_DOUBLE(0.0   , MathLib::toDoubleNumber("-0.0"));
        ASSERT_EQUALS_DOUBLE(0.0   , MathLib::toDoubleNumber("+0.0"));
        ASSERT_EQUALS_DOUBLE('0'   , MathLib::toDoubleNumber("'0'"));

        // verify: string --> double --> string conversion
        ASSERT_EQUALS("1.0" , MathLib::toString(MathLib::toDoubleNumber("1.0f")));
        ASSERT_EQUALS("1.0" , MathLib::toString(MathLib::toDoubleNumber("1.0")));
        ASSERT_EQUALS("0.0" , MathLib::toString(MathLib::toDoubleNumber("0.0f")));
        ASSERT_EQUALS("0.0" , MathLib::toString(MathLib::toDoubleNumber("0.0")));
        ASSERT_EQUALS("-1.0" , MathLib::toString(MathLib::toDoubleNumber("-1.0f")));
        ASSERT_EQUALS("-1.0" , MathLib::toString(MathLib::toDoubleNumber("-1.0")));
        ASSERT_EQUALS("0.0" , MathLib::toString(MathLib::toDoubleNumber("-0.0f")));
        ASSERT_EQUALS("0.0" , MathLib::toString(MathLib::toDoubleNumber("-0.0")));
        ASSERT_EQUALS("1.0" , MathLib::toString(MathLib::toDoubleNumber("+1.0f")));
        ASSERT_EQUALS("1.0" , MathLib::toString(MathLib::toDoubleNumber("+1.0")));
        ASSERT_EQUALS("0.0" , MathLib::toString(MathLib::toDoubleNumber("+0.0f")));
        ASSERT_EQUALS("0.0" , MathLib::toString(MathLib::toDoubleNumber("+0.0")));
        ASSERT_EQUALS("0.0" , MathLib::toString(MathLib::toDoubleNumber("-0")));
        ASSERT_EQUALS("0.0" , MathLib::toString(MathLib::toDoubleNumber("-0.")));
        ASSERT_EQUALS("0.0" , MathLib::toString(MathLib::toDoubleNumber("-0.0")));
    }

    void isint() const {
        // zero tests
        ASSERT_EQUALS(true , MathLib::isInt("0"));
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

        ASSERT_EQUALS(true , MathLib::isInt("1"));
        ASSERT_EQUALS(true , MathLib::isInt("-1"));
        ASSERT_EQUALS(true , MathLib::isInt("+1"));
        ASSERT_EQUALS(false , MathLib::isInt("+1E+1"));
        ASSERT_EQUALS(false , MathLib::isInt("+1E+10000"));
        ASSERT_EQUALS(false , MathLib::isInt("-1E+1"));
        ASSERT_EQUALS(false , MathLib::isInt("-1E+10000"));
        ASSERT_EQUALS(false, MathLib::isInt("-1E-1"));
        ASSERT_EQUALS(false, MathLib::isInt("-1E-10000"));

        ASSERT_EQUALS(true , MathLib::isInt("0xff"));
        ASSERT_EQUALS(true , MathLib::isInt("0xa"));
        ASSERT_EQUALS(true , MathLib::isInt("0b1000"));
        ASSERT_EQUALS(true , MathLib::isInt("0B1000"));
        ASSERT_EQUALS(true , MathLib::isInt("0l"));
        ASSERT_EQUALS(true , MathLib::isInt("0L"));
        ASSERT_EQUALS(true , MathLib::isInt("0ul"));
        ASSERT_EQUALS(true , MathLib::isInt("0ull"));
        ASSERT_EQUALS(true , MathLib::isInt("0llu"));
        ASSERT_EQUALS(true , MathLib::isInt("333L"));
        ASSERT_EQUALS(true , MathLib::isInt("330L"));
        ASSERT_EQUALS(true , MathLib::isInt("330llu"));
        ASSERT_EQUALS(true , MathLib::isInt("07"));
        ASSERT_EQUALS(true , MathLib::isInt("0123"));
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
        ASSERT_EQUALS(true , MathLib::isNegative("-1"));
        ASSERT_EQUALS(true , MathLib::isNegative("-1."));
        ASSERT_EQUALS(true , MathLib::isNegative("-1.0"));
        ASSERT_EQUALS(true , MathLib::isNegative("-1.0E+2"));
        ASSERT_EQUALS(true , MathLib::isNegative("-1.0E-2"));

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
        ASSERT_EQUALS(false, MathLib::isFloatHex(""));
        ASSERT_EQUALS(true, MathLib::isFloatHex("0x1.999999999999ap-4"));
        ASSERT_EQUALS(true, MathLib::isFloatHex("0x0.3p10"));
        ASSERT_EQUALS(true, MathLib::isFloatHex("0x1.fp3"));
        ASSERT_EQUALS(true, MathLib::isFloatHex("0x1P-1"));
        ASSERT_EQUALS(true, MathLib::isFloatHex("0xcc.ccccccccccdp-11"));
        ASSERT_EQUALS(true, MathLib::isFloatHex("0x3.243F6A88p+03"));
        ASSERT_EQUALS(true, MathLib::isFloatHex("0xA.Fp-10"));

        ASSERT_EQUALS(false, MathLib::isFloatHex("0"));
        ASSERT_EQUALS(false, MathLib::isFloatHex("0x"));
        ASSERT_EQUALS(false, MathLib::isFloatHex("0xa"));
        ASSERT_EQUALS(false, MathLib::isFloatHex("+0x"));
        ASSERT_EQUALS(false, MathLib::isFloatHex("-0x"));
        ASSERT_EQUALS(false, MathLib::isFloatHex("0x"));
        ASSERT_EQUALS(false, MathLib::isFloatHex("0x."));
        ASSERT_EQUALS(false, MathLib::isFloatHex("0XP"));
        ASSERT_EQUALS(false, MathLib::isFloatHex("0xx"));
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

        // negative testing
        ASSERT_EQUALS(false, MathLib::isIntHex("+0x"));
        ASSERT_EQUALS(false, MathLib::isIntHex("-0x"));
        ASSERT_EQUALS(false, MathLib::isIntHex("0x"));
        ASSERT_EQUALS(false, MathLib::isIntHex("0xx"));
        ASSERT_EQUALS(false, MathLib::isIntHex("-0175"));
        ASSERT_EQUALS(false, MathLib::isIntHex("-0_garbage_"));
        ASSERT_EQUALS(false, MathLib::isIntHex("  "));
        ASSERT_EQUALS(false, MathLib::isIntHex(" "));
        ASSERT_EQUALS(false, MathLib::isIntHex("0"));
        ASSERT_EQUALS(false, MathLib::isIntHex("+0x0Z"));
        ASSERT_EQUALS(false, MathLib::isIntHex("-0x0Z"));
        ASSERT_EQUALS(false, MathLib::isIntHex("+0x0Uz"));
        ASSERT_EQUALS(false, MathLib::isIntHex("-0x0Uz"));
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

    void isValidIntegerSuffix(void) const {
        // negative testing
        std::string value = "ux";
        ASSERT_EQUALS(false, MathLib::isValidIntegerSuffix(value.begin(), value.end()));

        value = "ulx";
        ASSERT_EQUALS(false, MathLib::isValidIntegerSuffix(value.begin(), value.end()));

        value = "lx";
        ASSERT_EQUALS(false, MathLib::isValidIntegerSuffix(value.begin(), value.end()));

        value = "lux";
        ASSERT_EQUALS(false, MathLib::isValidIntegerSuffix(value.begin(), value.end()));

        value = "lll";
        ASSERT_EQUALS(false, MathLib::isValidIntegerSuffix(value.begin(), value.end()));

        value = "garbage";
        ASSERT_EQUALS(false, MathLib::isValidIntegerSuffix(value.begin(), value.end()));

        value = "";
        ASSERT_EQUALS(false, MathLib::isValidIntegerSuffix(value.begin(), value.end()));

        value = "llu ";
        ASSERT_EQUALS(false, MathLib::isValidIntegerSuffix(value.begin(), value.end()));

        value = "i";
        ASSERT_EQUALS(false, MathLib::isValidIntegerSuffix(value.begin(), value.end()));

        value = "iX";
        ASSERT_EQUALS(false, MathLib::isValidIntegerSuffix(value.begin(), value.end()));

        value = "i6X";
        ASSERT_EQUALS(false, MathLib::isValidIntegerSuffix(value.begin(), value.end()));

        value = "i64X";
        ASSERT_EQUALS(false, MathLib::isValidIntegerSuffix(value.begin(), value.end()));

        value = "i64 ";
        ASSERT_EQUALS(false, MathLib::isValidIntegerSuffix(value.begin(), value.end()));

        value = "i66";
        ASSERT_EQUALS(false, MathLib::isValidIntegerSuffix(value.begin(), value.end()));

        // positive testing
        value = "u";
        ASSERT_EQUALS(true, MathLib::isValidIntegerSuffix(value.begin(), value.end()));

        value = "ul";
        ASSERT_EQUALS(true, MathLib::isValidIntegerSuffix(value.begin(), value.end()));

        value = "ull";
        ASSERT_EQUALS(true, MathLib::isValidIntegerSuffix(value.begin(), value.end()));

        value = "l";
        ASSERT_EQUALS(true, MathLib::isValidIntegerSuffix(value.begin(), value.end()));

        value = "lu";
        ASSERT_EQUALS(true, MathLib::isValidIntegerSuffix(value.begin(), value.end()));

        value = "ll";
        ASSERT_EQUALS(true, MathLib::isValidIntegerSuffix(value.begin(), value.end()));

        value = "llu";
        ASSERT_EQUALS(true, MathLib::isValidIntegerSuffix(value.begin(), value.end()));

        value = "i64";
        ASSERT_EQUALS(true, MathLib::isValidIntegerSuffix(value.begin(), value.end()));

        value = "ui64";
        ASSERT_EQUALS(true, MathLib::isValidIntegerSuffix(value.begin(), value.end()));
    }

    void ispositive() const {
        ASSERT_EQUALS(false, MathLib::isPositive("-1"));
        ASSERT_EQUALS(false, MathLib::isPositive("-1."));
        ASSERT_EQUALS(false, MathLib::isPositive("-1.0"));
        ASSERT_EQUALS(false, MathLib::isPositive("-1.0E+2"));
        ASSERT_EQUALS(false, MathLib::isPositive("-1.0E-2"));

        ASSERT_EQUALS(true , MathLib::isPositive("+1"));
        ASSERT_EQUALS(true , MathLib::isPositive("+1."));
        ASSERT_EQUALS(true , MathLib::isPositive("+1.0"));
        ASSERT_EQUALS(true , MathLib::isPositive("+1.0E+2"));
        ASSERT_EQUALS(true , MathLib::isPositive("+1.0E-2"));

        // test empty string
        ASSERT_EQUALS(false, MathLib::isPositive("")); // "" is neither positive nor negative
    }

    void isFloat() const {
        ASSERT_EQUALS(false, MathLib::isFloat(""));
        ASSERT_EQUALS(true , MathLib::isFloat("0.f"));
        ASSERT_EQUALS(true , MathLib::isFloat("0.f"));
        ASSERT_EQUALS(true , MathLib::isFloat("0xA.Fp-10"));
    }

    void isDecimalFloat() const {
        ASSERT_EQUALS(false, MathLib::isDecimalFloat(""));
        ASSERT_EQUALS(false, MathLib::isDecimalFloat("."));
        ASSERT_EQUALS(false, MathLib::isDecimalFloat("..."));
        ASSERT_EQUALS(false, MathLib::isDecimalFloat(".e"));
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

        ASSERT_EQUALS(true , MathLib::isDecimalFloat("0."));
        ASSERT_EQUALS(true , MathLib::isDecimalFloat("0.f"));
        ASSERT_EQUALS(true , MathLib::isDecimalFloat("0.F"));
        ASSERT_EQUALS(true , MathLib::isDecimalFloat("0.l"));
        ASSERT_EQUALS(true , MathLib::isDecimalFloat("0.L"));
        ASSERT_EQUALS(false , MathLib::isDecimalFloat("0. "));
        ASSERT_EQUALS(false , MathLib::isDecimalFloat(" 0. "));
        ASSERT_EQUALS(false , MathLib::isDecimalFloat(" 0."));

        ASSERT_EQUALS(false , MathLib::isDecimalFloat("0.."));
        ASSERT_EQUALS(false , MathLib::isDecimalFloat("..0.."));
        ASSERT_EQUALS(false , MathLib::isDecimalFloat("..0"));
        ASSERT_EQUALS(true , MathLib::isDecimalFloat("0.0"));
        ASSERT_EQUALS(true , MathLib::isDecimalFloat("0.0f"));
        ASSERT_EQUALS(true , MathLib::isDecimalFloat("0.0F"));
        ASSERT_EQUALS(true , MathLib::isDecimalFloat("0.0l"));
        ASSERT_EQUALS(true , MathLib::isDecimalFloat("0.0L"));
        ASSERT_EQUALS(true , MathLib::isDecimalFloat("-0."));
        ASSERT_EQUALS(true , MathLib::isDecimalFloat("+0."));
        ASSERT_EQUALS(true , MathLib::isDecimalFloat("-0.0"));
        ASSERT_EQUALS(true , MathLib::isDecimalFloat("+0.0"));
        ASSERT_EQUALS(true , MathLib::isDecimalFloat("0E0"));
        ASSERT_EQUALS(true , MathLib::isDecimalFloat("+0E0"));
        ASSERT_EQUALS(true , MathLib::isDecimalFloat("+0E0"));
        ASSERT_EQUALS(true , MathLib::isDecimalFloat("+0E+0"));
        ASSERT_EQUALS(true , MathLib::isDecimalFloat("+0E-0"));
        ASSERT_EQUALS(true , MathLib::isDecimalFloat("-0E+0"));
        ASSERT_EQUALS(true , MathLib::isDecimalFloat("-0E-0"));
        ASSERT_EQUALS(true , MathLib::isDecimalFloat("+0.0E+1"));
        ASSERT_EQUALS(true , MathLib::isDecimalFloat("+0.0E-1"));
        ASSERT_EQUALS(true , MathLib::isDecimalFloat("-0.0E+1"));
        ASSERT_EQUALS(true , MathLib::isDecimalFloat("-0.0E-1"));

        ASSERT_EQUALS(false , MathLib::isDecimalFloat("1"));
        ASSERT_EQUALS(false , MathLib::isDecimalFloat("-1"));
        ASSERT_EQUALS(false , MathLib::isDecimalFloat("+1"));
        ASSERT_EQUALS(true , MathLib::isDecimalFloat("+1e+1"));
        ASSERT_EQUALS(true , MathLib::isDecimalFloat("+1E+1"));
        ASSERT_EQUALS(true , MathLib::isDecimalFloat("+1E+100"));
        ASSERT_EQUALS(true , MathLib::isDecimalFloat("+1E+100f"));
        ASSERT_EQUALS(true , MathLib::isDecimalFloat("+1E+100F"));
        ASSERT_EQUALS(true , MathLib::isDecimalFloat("+1E+100l"));
        ASSERT_EQUALS(true , MathLib::isDecimalFloat("+1E+100L"));
        ASSERT_EQUALS(true , MathLib::isDecimalFloat("+1E+007")); // to be sure about #5485
        ASSERT_EQUALS(true , MathLib::isDecimalFloat("+1E+001f"));
        ASSERT_EQUALS(true , MathLib::isDecimalFloat("+1E+001F"));
        ASSERT_EQUALS(true , MathLib::isDecimalFloat("+1E+001l"));
        ASSERT_EQUALS(true , MathLib::isDecimalFloat("+1E+001L"));
        ASSERT_EQUALS(true , MathLib::isDecimalFloat("+1E+10000"));
        ASSERT_EQUALS(true , MathLib::isDecimalFloat("-1E+1"));
        ASSERT_EQUALS(true , MathLib::isDecimalFloat("-1E+10000"));
        ASSERT_EQUALS(true , MathLib::isDecimalFloat(".1250E+04"));
        ASSERT_EQUALS(true , MathLib::isDecimalFloat("-1E-1"));
        ASSERT_EQUALS(true , MathLib::isDecimalFloat("-1E-10000"));
        ASSERT_EQUALS(true , MathLib::isDecimalFloat("+1.23e+01"));
        ASSERT_EQUALS(true , MathLib::isDecimalFloat("+1.23E+01"));
        ASSERT_EQUALS(false , MathLib::isDecimalFloat("+1e+x"));
        ASSERT_EQUALS(false , MathLib::isDecimalFloat("+1E+X"));
        ASSERT_EQUALS(false , MathLib::isDecimalFloat("+1E+001lX"));
        ASSERT_EQUALS(false , MathLib::isDecimalFloat("+1E+001LX"));
        ASSERT_EQUALS(false , MathLib::isDecimalFloat("+1E+001f2"));
        ASSERT_EQUALS(false , MathLib::isDecimalFloat("+1E+001F2"));
        ASSERT_EQUALS(false , MathLib::isDecimalFloat("+1e+003x"));
        ASSERT_EQUALS(false , MathLib::isDecimalFloat("+1E+003X"));


        ASSERT_EQUALS(true , MathLib::isDecimalFloat("0.4"));
        ASSERT_EQUALS(true , MathLib::isDecimalFloat("2352.3f"));
        ASSERT_EQUALS(true , MathLib::isDecimalFloat("2352.3F"));
        ASSERT_EQUALS(true , MathLib::isDecimalFloat("2352.3l"));
        ASSERT_EQUALS(true , MathLib::isDecimalFloat("2352.3L"));
        ASSERT_EQUALS(true , MathLib::isDecimalFloat("0.00004"));
        ASSERT_EQUALS(true , MathLib::isDecimalFloat("2352.00001f"));
        ASSERT_EQUALS(true , MathLib::isDecimalFloat("2352.00001F"));
        ASSERT_EQUALS(true , MathLib::isDecimalFloat("2352.00001l"));
        ASSERT_EQUALS(true , MathLib::isDecimalFloat("2352.00001L"));
        ASSERT_EQUALS(true , MathLib::isDecimalFloat(".4"));
        ASSERT_EQUALS(true , MathLib::isDecimalFloat(".3e2"));
        ASSERT_EQUALS(true , MathLib::isDecimalFloat("1.0E+1"));
        ASSERT_EQUALS(true , MathLib::isDecimalFloat("1.0E-1"));
        ASSERT_EQUALS(true , MathLib::isDecimalFloat("-1.0E+1"));
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
        ASSERT_EQUALS("-inf.0", MathLib::divide("-3.0", "-0.0f")); // inf (#5142)
    }

    void isdec(void) const {
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
        ASSERT_EQUALS(false, MathLib::isNullValue("-00.01e-12"));
        ASSERT_EQUALS(false, MathLib::isNullValue("-00.01e+12"));
        ASSERT_EQUALS(false, MathLib::isNullValue(""));
        ASSERT_EQUALS(false, MathLib::isNullValue(" "));
        ASSERT_EQUALS(false, MathLib::isNullValue("x"));
        ASSERT_EQUALS(false, MathLib::isNullValue("garbage"));
        ASSERT_EQUALS(false, MathLib::isNullValue("UL"));
    }

    void incdec() const {
        // increment
        {
            MathLib::biguint num = ~10U;
            const std::string op = "++";
            const std::string strNum = MathLib::incdec(MathLib::toString(num), op);
            const MathLib::biguint incrementedNum = MathLib::toULongNumber(strNum);
            ASSERT_EQUALS(num + 1U, incrementedNum);
        }
        // decrement
        {
            MathLib::biguint num = ~10U;
            const std::string op = "--";
            const std::string strNum = MathLib::incdec(MathLib::toString(num), op);
            const MathLib::biguint decrementedNum = MathLib::toULongNumber(strNum);
            ASSERT_EQUALS(num - 1U, decrementedNum);
        }
        // invalid operation
        ASSERT_THROW(MathLib::incdec("1", "x"), InternalError); // throw
    }

    void sin() const {
        ASSERT_EQUALS("0.0"   , MathLib::sin("0"));
    }
    void cos() const {
        ASSERT_EQUALS("1.0"   , MathLib::cos("0"));
    }
    void tan() const {
        ASSERT_EQUALS("0.0"   , MathLib::tan("0"));
    }
    void abs() const {
        ASSERT_EQUALS("0.0"   , MathLib::abs("0"));
        ASSERT_EQUALS("0.0"   , MathLib::abs("+0"));
        ASSERT_EQUALS("0.0"   , MathLib::abs("-0"));
        ASSERT_EQUALS("1.0"   , MathLib::abs("+1"));
        ASSERT_EQUALS("1.0"   , MathLib::abs("+1.0"));
        ASSERT_EQUALS("1.0"   , MathLib::abs("-1"));
        ASSERT_EQUALS("1.0"   , MathLib::abs("-1.0"));
    }

    void toString() const {
        ASSERT_EQUALS("0.0"   , MathLib::toString(0.0));
        ASSERT_EQUALS("0.0"   , MathLib::toString(+0.0));
        ASSERT_EQUALS("0.0"   , MathLib::toString(-0.0));
        // float (trailing f or F)
        ASSERT_EQUALS("0"     , MathLib::toString(+0.0f));
        ASSERT_EQUALS("-0"    , MathLib::toString(-0.0F));
        // double (tailing l or L)
        ASSERT_EQUALS("0"     , MathLib::toString(+0.0l));
        ASSERT_EQUALS("-0"    , MathLib::toString(-0.0L));
    }

    void characterLiteralsNormalization() const {
        // `A` is 0x41 and 0101
        ASSERT_EQUALS("A" , MathLib::normalizeCharacterLiteral("\\x41"));
        ASSERT_EQUALS("A" , MathLib::normalizeCharacterLiteral("\\101"));
        // Hexa and octal numbers should not only be intepreted in byte 1
        ASSERT_EQUALS("TESTATEST" , MathLib::normalizeCharacterLiteral("TEST\\x41TEST"));
        ASSERT_EQUALS("TESTATEST" , MathLib::normalizeCharacterLiteral("TEST\\101TEST"));
        ASSERT_EQUALS("TESTTESTA" , MathLib::normalizeCharacterLiteral("TESTTEST\\x41"));
        ASSERT_EQUALS("TESTTESTA" , MathLib::normalizeCharacterLiteral("TESTTEST\\101"));
        // Single escape sequences
        ASSERT_EQUALS("\?" , MathLib::normalizeCharacterLiteral("\\?"));
        ASSERT_EQUALS("\'" , MathLib::normalizeCharacterLiteral("\\'"));
        // Incomplete hexa and octal sequences
        ASSERT_THROW(MathLib::normalizeCharacterLiteral("\\"), InternalError);
        ASSERT_THROW(MathLib::normalizeCharacterLiteral("\\x"), InternalError);
        // No octal digit in an octal sequence
        ASSERT_THROW(MathLib::normalizeCharacterLiteral("\\9"), InternalError);
        // Unsupported single escape sequence
        ASSERT_THROW(MathLib::normalizeCharacterLiteral("\\c"), InternalError);
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
