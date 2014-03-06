/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2014 Daniel Marjamäki and Cppcheck team.
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
        TEST_CASE(calculate);
        TEST_CASE(calculate1);
        TEST_CASE(convert);
        TEST_CASE(isint);
        TEST_CASE(isnegative);
        TEST_CASE(ispositive);
        TEST_CASE(isfloat);
        TEST_CASE(isGreater)
        TEST_CASE(isGreaterEqual)
        TEST_CASE(isEqual)
        TEST_CASE(isNotEqual)
        TEST_CASE(isLess)
        TEST_CASE(isLessEqual)
        TEST_CASE(naninf)
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
        MathLib::divide("123", "0.0"); // don't throw

        // Unknown action should throw exception
        ASSERT_THROW(MathLib::calculate("1","2",'j'),InternalError);
    }

    void calculate1() const { // mod
        ASSERT_EQUALS("0"    , MathLib::calculate("2"    , "1"    , '%'));
        ASSERT_EQUALS("0.0"  , MathLib::calculate("2.0"  , "1.0"  , '%'));
        ASSERT_EQUALS("2"    , MathLib::calculate("12"   , "5"   , '%'));
        ASSERT_EQUALS("1"    , MathLib::calculate("100"  , "3"   , '%'));
        ASSERT_EQUALS("12.0" , MathLib::calculate("12.0" , "13.0" , '%'));
        ASSERT_EQUALS("1.3"  , MathLib::calculate("5.3"  , "2.0" , '%'));
        ASSERT_EQUALS("1.7"  , MathLib::calculate("18.5" , "4.2" , '%'));
        ASSERT_THROW(MathLib::calculate("123", "0", '%'), InternalError); // throw
        MathLib::calculate("123", "0.0", '%'); // don't throw
    }

    void convert() const {
        // ------------------
        // tolong conversion:
        // ------------------

        // from hex
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

        // from long long
        ASSERT_EQUALS(0xFF00000000000000LL, MathLib::toLongNumber("0xFF00000000000000LL"));
        ASSERT_EQUALS(0x0A00000000000000LL, MathLib::toLongNumber("0x0A00000000000000LL"));

        // -----------------
        // to double number:
        // -----------------
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
    }

    void isfloat() const {
        ASSERT_EQUALS(false, MathLib::isFloat(""));
        ASSERT_EQUALS(false, MathLib::isFloat("."));
        ASSERT_EQUALS(false, MathLib::isFloat("..."));
        ASSERT_EQUALS(false, MathLib::isFloat("+E."));
        ASSERT_EQUALS(false, MathLib::isFloat("+e."));
        ASSERT_EQUALS(false, MathLib::isFloat("-E."));
        ASSERT_EQUALS(false, MathLib::isFloat("-e."));
        ASSERT_EQUALS(false, MathLib::isFloat("-."));
        ASSERT_EQUALS(false, MathLib::isFloat("-."));
        ASSERT_EQUALS(false, MathLib::isFloat("-"));
        ASSERT_EQUALS(false, MathLib::isFloat("+"));
        ASSERT_EQUALS(false, MathLib::isFloat(" "));

        ASSERT_EQUALS(false, MathLib::isFloat("0"));
        ASSERT_EQUALS(false, MathLib::isFloat("0 "));
        ASSERT_EQUALS(false, MathLib::isFloat(" 0 "));
        ASSERT_EQUALS(false, MathLib::isFloat(" 0"));

        ASSERT_EQUALS(true , MathLib::isFloat("0."));
        ASSERT_EQUALS(false , MathLib::isFloat("0. "));
        ASSERT_EQUALS(false , MathLib::isFloat(" 0. "));
        ASSERT_EQUALS(false , MathLib::isFloat(" 0."));

        ASSERT_EQUALS(false , MathLib::isFloat("0.."));
        ASSERT_EQUALS(false , MathLib::isFloat("..0.."));
        ASSERT_EQUALS(false , MathLib::isFloat("..0"));
        ASSERT_EQUALS(true , MathLib::isFloat("0.0"));
        ASSERT_EQUALS(true , MathLib::isFloat("-0."));
        ASSERT_EQUALS(true , MathLib::isFloat("+0."));
        ASSERT_EQUALS(true , MathLib::isFloat("-0.0"));
        ASSERT_EQUALS(true , MathLib::isFloat("+0.0"));
        ASSERT_EQUALS(true , MathLib::isFloat("0E0"));
        ASSERT_EQUALS(true , MathLib::isFloat("+0E0"));
        ASSERT_EQUALS(true , MathLib::isFloat("+0E0"));
        ASSERT_EQUALS(true , MathLib::isFloat("+0E+0"));
        ASSERT_EQUALS(true , MathLib::isFloat("+0E-0"));
        ASSERT_EQUALS(true , MathLib::isFloat("-0E+0"));
        ASSERT_EQUALS(true , MathLib::isFloat("-0E-0"));
        ASSERT_EQUALS(true , MathLib::isFloat("+0.0E+1"));
        ASSERT_EQUALS(true , MathLib::isFloat("+0.0E-1"));
        ASSERT_EQUALS(true , MathLib::isFloat("-0.0E+1"));
        ASSERT_EQUALS(true , MathLib::isFloat("-0.0E-1"));

        ASSERT_EQUALS(false , MathLib::isFloat("1"));
        ASSERT_EQUALS(false , MathLib::isFloat("-1"));
        ASSERT_EQUALS(false , MathLib::isFloat("+1"));
        ASSERT_EQUALS(true , MathLib::isFloat("+1e+1"));
        ASSERT_EQUALS(true , MathLib::isFloat("+1E+1"));
        ASSERT_EQUALS(true , MathLib::isFloat("+1E+100"));
        ASSERT_EQUALS(true , MathLib::isFloat("+1E+100f"));
        ASSERT_EQUALS(true , MathLib::isFloat("+1E+007")); // to be sure about #5485
        ASSERT_EQUALS(true , MathLib::isFloat("+1E+001f"));
        ASSERT_EQUALS(false , MathLib::isFloat("+1E+001f2"));
        ASSERT_EQUALS(true , MathLib::isFloat("+1E+10000"));
        ASSERT_EQUALS(true , MathLib::isFloat("-1E+1"));
        ASSERT_EQUALS(true , MathLib::isFloat("-1E+10000"));
        ASSERT_EQUALS(true , MathLib::isFloat(".1250E+04"));
        ASSERT_EQUALS(true , MathLib::isFloat("-1E-1"));
        ASSERT_EQUALS(true , MathLib::isFloat("-1E-10000"));
        ASSERT_EQUALS(true , MathLib::isFloat("+1.23e+01"));
        ASSERT_EQUALS(true , MathLib::isFloat("+1.23E+01"));

        ASSERT_EQUALS(true , MathLib::isFloat("0.4"));
        ASSERT_EQUALS(true , MathLib::isFloat("2352.3f"));
        ASSERT_EQUALS(true , MathLib::isFloat("0.00004"));
        ASSERT_EQUALS(true , MathLib::isFloat("2352.00001f"));
        ASSERT_EQUALS(true , MathLib::isFloat(".4"));
        ASSERT_EQUALS(true , MathLib::isFloat(".3e2"));
        ASSERT_EQUALS(true , MathLib::isFloat("1.0E+1"));
        ASSERT_EQUALS(true , MathLib::isFloat("1.0E-1"));
        ASSERT_EQUALS(true , MathLib::isFloat("-1.0E+1"));
    }

    void naninf() {
        ASSERT_EQUALS("nan.0", MathLib::divide("0.0", "0.0")); // nan
        ASSERT_EQUALS("inf.0", MathLib::divide("3.0", "0.0")); // inf
        ASSERT_EQUALS("-inf.0", MathLib::divide("-3.0", "0.0")); // -inf (#5142)
    }
};

REGISTER_TEST(TestMathLib)
