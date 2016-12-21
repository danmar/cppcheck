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

#include "preprocessor.h"
#include "tokenize.h"
#include "symboldatabase.h"
#include "checktype.h"
#include "testsuite.h"
#include "testutils.h"
#include <tinyxml2.h>


class TestType : public TestFixture {
public:
    TestType() : TestFixture("TestType") {
    }

private:


    void run() {
        TEST_CASE(checkTooBigShift);
        TEST_CASE(checkIntegerOverflow);
        TEST_CASE(signConversion);
        TEST_CASE(longCastAssign);
        TEST_CASE(longCastReturn);
        TEST_CASE(checkFloatToIntegerOverflow);
    }

    void check(const char code[], Settings* settings = 0, const char filename[] = "test.cpp") {
        // Clear the error buffer..
        errout.str("");

        if (!settings) {
            static Settings _settings;
            settings = &_settings;
        }
        settings->addEnabled("warning");

        // Tokenize..
        Tokenizer tokenizer(settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, filename);

        // Check..
        CheckType checkType(&tokenizer, settings, this);
        checkType.runChecks(&tokenizer, settings, this);
    }

    void checkTooBigShift() {
        Settings settings;
        settings.platform(Settings::Unix32);

        check("int foo(int x) {\n"
              "   return x << 32;\n"
              "}",&settings);
        ASSERT_EQUALS("[test.cpp:2]: (error) Shifting 32-bit value by 32 bits is undefined behaviour\n", errout.str());

        check("int foo(int x) {\n"
              "   return x << 2;\n"
              "}",&settings);
        ASSERT_EQUALS("", errout.str());

        check("int foo(int x) {\n"
              "   return (long long)x << 40;\n"
              "}",&settings);
        ASSERT_EQUALS("", errout.str());

        check("void foo() {\n"
              "  QList<int> someList;\n"
              "  someList << 300;\n"
              "}", &settings);
        ASSERT_EQUALS("", errout.str());

        // Ticket #6793
        check("template<int I> int foo(int x) { return x << I; }\n"
              "const int f = foo<31>(0);\n"
              "const int g = foo<100>(0);\n"
              "template<int I> int hoo(int x) { return x << 32; }\n"
              "const int h = hoo<100>(0);", &settings);
        ASSERT_EQUALS("[test.cpp:4]: (error) Shifting 32-bit value by 32 bits is undefined behaviour\n"
                      "[test.cpp:1]: (error) Shifting 32-bit value by 100 bits is undefined behaviour\n", errout.str());

        // #7266: C++, shift in macro
        check("void f(int x) {\n"
              "    UINFO(x << 1234);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void checkIntegerOverflow() {
        Settings settings;
        settings.platform(Settings::Unix32);
        settings.addEnabled("warning");

        check("void foo() {\n"
              "    int intmax = 0x7fffffff;\n"
              "    return intmax + 1;\n"
              "}",&settings);
        ASSERT_EQUALS("[test.cpp:3]: (error) Signed integer overflow for expression 'intmax+1'.\n", errout.str());

        check("void foo() {\n"
              "    int intmax = 0x7fffffff;\n"
              "    return intmax - 1;\n"
              "}",&settings);
        ASSERT_EQUALS("", errout.str());

        check("int foo(signed int x) {\n"
              "   if (x==123456) {}\n"
              "   return x * x;\n"
              "}",&settings);
        ASSERT_EQUALS("[test.cpp:3]: (warning) Either the condition 'x==123456' is redundant or there is signed integer overflow for expression 'x*x'.\n", errout.str());

        check("int foo(signed int x) {\n"
              "   if (x==123456) {}\n"
              "   return -123456 * x;\n"
              "}",&settings);
        ASSERT_EQUALS("[test.cpp:3]: (warning) Either the condition 'x==123456' is redundant or there is signed integer overflow for expression '-123456*x'.\n", errout.str());

        check("int foo(signed int x) {\n"
              "   if (x==123456) {}\n"
              "   return 123456U * x;\n"
              "}",&settings);
        ASSERT_EQUALS("", errout.str());

        check("int foo() {\n"
              "  x = 1 << 31;\n" // this is technically integer overflow but it's common code
              "}", &settings, "test.c");
        ASSERT_EQUALS("", errout.str());
    }

    void signConversion() {
        check("unsigned int f1(signed int x, unsigned int y) {" // x is signed
              "  return x * y;\n"
              "}\n"
              "void f2() { f1(-4,4); }");
        ASSERT_EQUALS("[test.cpp:1]: (warning) Suspicious code: sign conversion of x in calculation, even though x can have a negative value\n", errout.str());

        check("unsigned int f1(int x) {" // x has no signedness, but it can have the value -1 so assume it's signed
              "  return x * 5U;\n"
              "}\n"
              "void f2() { f1(-4); }");
        ASSERT_EQUALS("[test.cpp:1]: (warning) Suspicious code: sign conversion of x in calculation, even though x can have a negative value\n", errout.str());

        check("unsigned int f1(int x) {" // #6168: FP for inner calculation
              "  return 5U * (1234 - x);\n" // <- signed subtraction, x is not sign converted
              "}\n"
              "void f2() { f1(-4); }");
        ASSERT_EQUALS("", errout.str());

        // Don't warn for + and -
        check("void f1(int x) {"
              "  a = x + 5U;\n"
              "}\n"
              "void f2() { f1(-4); }");
        ASSERT_EQUALS("", errout.str());

        check("size_t foo(size_t x) {\n"
              " return -2 * x;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Suspicious code: sign conversion of -2 in calculation because '-2' has a negative value\n", errout.str());
    }

    void longCastAssign() {
        Settings settings;
        settings.addEnabled("style");
        settings.platform(Settings::Unix64);

        check("long f(int x, int y) {\n"
              "  const long ret = x * y;\n"
              "  return ret;\n"
              "}\n", &settings);
        ASSERT_EQUALS("[test.cpp:2]: (style) int result is assigned to long variable. If the variable is long to avoid loss of information, then you have loss of information.\n", errout.str());

        // typedef
        check("long f(int x, int y) {\n"
              "  const size_t ret = x * y;\n"
              "  return ret;\n"
              "}\n", &settings);
        ASSERT_EQUALS("", errout.str());

        // astIsIntResult
        check("long f(int x, int y) {\n"
              "  const long ret = (long)x * y;\n"
              "  return ret;\n"
              "}\n", &settings);
        ASSERT_EQUALS("", errout.str());
    }

    void longCastReturn() {
        Settings settings;
        settings.addEnabled("style");

        check("long f(int x, int y) {\n"
              "  return x * y;\n"
              "}\n", &settings);
        ASSERT_EQUALS("[test.cpp:2]: (style) int result is returned as long value. If the return value is long to avoid loss of information, then you have loss of information.\n", errout.str());

        // typedef
        check("size_t f(int x, int y) {\n"
              "  return x * y;\n"
              "}\n", &settings);
        ASSERT_EQUALS("", errout.str());
    }

    // This function ensure that test works with different compilers. Floats can
    // be stringified differently.
    std::string removeFloat(const std::string& msg) {
        std::string::size_type pos1 = msg.find("float (");
        std::string::size_type pos2 = msg.find(") conversion");
        if (pos1 == std::string::npos || pos2 == std::string::npos || pos1 > pos2)
            return msg;
        return msg.substr(0,pos1+7) + msg.substr(pos2);
    }

    void checkFloatToIntegerOverflow() {
        check("void f(void) {\n"
              "  return (int)1E100;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2]: (error) Undefined behaviour: float () conversion overflow.\n", removeFloat(errout.str()));

        check("void f(void) {\n"
              "  return (int)-1E100;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2]: (error) Undefined behaviour: float () conversion overflow.\n", removeFloat(errout.str()));

        check("void f(void) {\n"
              "  return (short)1E6;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2]: (error) Undefined behaviour: float () conversion overflow.\n", removeFloat(errout.str()));

        check("void f(void) {\n"
              "  return (unsigned char)256.0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2]: (error) Undefined behaviour: float () conversion overflow.\n", removeFloat(errout.str()));

        check("void f(void) {\n"
              "  return (unsigned char)255.5;\n"
              "}\n");
        ASSERT_EQUALS("", removeFloat(errout.str()));
    }
};

REGISTER_TEST(TestType)
