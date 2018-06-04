/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2018 Cppcheck team.
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

#include "checktype.h"
#include "platform.h"
#include "settings.h"
#include "testsuite.h"
#include "tokenize.h"

#include <string>

class TestType : public TestFixture {
public:
    TestType() : TestFixture("TestType") {
    }

private:


    void run() override {
        TEST_CASE(checkTooBigShift_Unix32);
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

    void checkTooBigShift_Unix32() {
        Settings settings;
        settings.platform(Settings::Unix32);

        // unsigned types getting promoted to int sizeof(int) = 4 bytes
        // and unsigned types having already a size of 4 bytes
        {
            const std::string type[6] = {"unsigned char", /*[unsigned]*/"char", "bool", "unsigned short", "unsigned int", "unsigned long"};
            for (short i = 0; i < 6U; ++i) {
                check((type[i] + " f(" + type[i] +" x) { return x << 33; }").c_str(),&settings);
                ASSERT_EQUALS("[test.cpp:1]: (error) Shifting 32-bit value by 33 bits is undefined behaviour\n", errout.str());
                check((type[i] + " f(int x) { return (x = (" + type[i] + ")x << 32); }").c_str(),&settings);
                ASSERT_EQUALS("[test.cpp:1]: (error) Shifting 32-bit value by 32 bits is undefined behaviour\n", errout.str());
                check((type[i] + " foo(" + type[i] + " x) { return x << 31; }").c_str(),&settings);
                ASSERT_EQUALS("", errout.str());
            }
        }
        // signed types getting promoted to int sizeof(int) = 4 bytes
        // and signed types having already a size of 4 bytes
        {
            const std::string type[7] = {"signed char", "signed short", /*[signed]*/"short", /*[signed]*/"int", "signed int", /*[signed]*/"long", "signed long"};
            for (short i = 0; i < 7U; ++i) {
                check((type[i] + " f(" + type[i] +" x) { return x << 33; }").c_str(),&settings);
                ASSERT_EQUALS("[test.cpp:1]: (error) Shifting 32-bit value by 33 bits is undefined behaviour\n", errout.str());
                check((type[i] + " f(int x) { return (x = (" + type[i] + ")x << 32); }").c_str(),&settings);
                ASSERT_EQUALS("[test.cpp:1]: (error) Shifting 32-bit value by 32 bits is undefined behaviour\n", errout.str());
                check((type[i] + " foo(" + type[i] + " x) { return x << 31; }").c_str(),&settings);
                ASSERT_EQUALS("[test.cpp:1]: (error) Shifting signed 32-bit value by 31 bits is undefined behaviour\n", errout.str());
                check((type[i] + " foo(" + type[i] + " x) { return x << 30; }").c_str(),&settings);
                ASSERT_EQUALS("", errout.str());
            }
        }
        // 64 bit width types
        {
            // unsigned long long
            check("unsigned long long foo(unsigned long long x) { return x << 64; }",&settings);
            ASSERT_EQUALS("[test.cpp:1]: (error) Shifting 64-bit value by 64 bits is undefined behaviour\n", errout.str());
            check("unsigned long long f(int x) { return (x = (unsigned long long)x << 64); }",&settings);
            ASSERT_EQUALS("[test.cpp:1]: (error) Shifting 64-bit value by 64 bits is undefined behaviour\n", errout.str());
            check("unsigned long long f(unsigned long long x) { return x << 63; }",&settings);
            ASSERT_EQUALS("", errout.str());
            // [signed] long long
            check("long long foo(long long x) { return x << 64; }",&settings);
            ASSERT_EQUALS("[test.cpp:1]: (error) Shifting 64-bit value by 64 bits is undefined behaviour\n", errout.str());
            check("long long f(int x) { return (x = (long long)x << 64); }",&settings);
            ASSERT_EQUALS("[test.cpp:1]: (error) Shifting 64-bit value by 64 bits is undefined behaviour\n", errout.str());
            check("long long f(long long x) { return x << 63; }",&settings);
            ASSERT_EQUALS("[test.cpp:1]: (error) Shifting signed 64-bit value by 63 bits is undefined behaviour\n", errout.str());
            check("long long f(long long x) { return x << 62; }",&settings);
            ASSERT_EQUALS("", errout.str());
            // signed long long
            check("signed long long foo(signed long long x) { return x << 64; }",&settings);
            ASSERT_EQUALS("[test.cpp:1]: (error) Shifting 64-bit value by 64 bits is undefined behaviour\n", errout.str());
            check("signed long long f(long long x) { return (x = (signed long long)x << 64); }",&settings);
            ASSERT_EQUALS("[test.cpp:1]: (error) Shifting 64-bit value by 64 bits is undefined behaviour\n", errout.str());
            check("signed long long f(signed long long x) { return x << 63; }",&settings);
            ASSERT_EQUALS("[test.cpp:1]: (error) Shifting signed 64-bit value by 63 bits is undefined behaviour\n", errout.str());
            check("signed long long f(signed long long x) { return x << 62; }",&settings);
            ASSERT_EQUALS("", errout.str());
        }

        check("void foo() {\n"
              "  QList<int> someList;\n"
              "  someList << 300;\n"
              "}", &settings);
        ASSERT_EQUALS("", errout.str());

        // Ticket #6793
        check("template<unsigned int I> int foo(unsigned int x) { return x << I; }\n"
              "const unsigned int f = foo<31>(0);\n"
              "const unsigned int g = foo<100>(0);\n"
              "template<unsigned int I> int hoo(unsigned int x) { return x << 32; }\n"
              "const unsigned int h = hoo<100>(0);", &settings);
        ASSERT_EQUALS("[test.cpp:4]: (error) Shifting 32-bit value by 32 bits is undefined behaviour\n"
                      "[test.cpp:1]: (error) Shifting 32-bit value by 100 bits is undefined behaviour\n", errout.str());

        // #7266: C++, shift in macro
        check("void f(unsigned int x) {\n"
              "    UINFO(x << 1234);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void checkIntegerOverflow() {
        Settings settings;
        settings.platform(Settings::Unix32);
        settings.addEnabled("warning");

        check("x = (int)0x10000 * (int)0x10000;", &settings);
        ASSERT_EQUALS("[test.cpp:1]: (error) Signed integer overflow for expression '(int)65536*(int)65536'.\n", errout.str());

        check("x = (long)0x10000 * (long)0x10000;", &settings);
        ASSERT_EQUALS("[test.cpp:1]: (error) Signed integer overflow for expression '(long)65536*(long)65536'.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (warning) Either the condition 'x==123456' is redundant or there is signed integer overflow for expression 'x*x'.\n", errout.str());

        check("int foo(signed int x) {\n"
              "   if (x==123456) {}\n"
              "   return -123456 * x;\n"
              "}",&settings);
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (warning) Either the condition 'x==123456' is redundant or there is signed integer overflow for expression '-123456*x'.\n", errout.str());

        check("int foo(signed int x) {\n"
              "   if (x==123456) {}\n"
              "   return 123456U * x;\n"
              "}",&settings);
        ASSERT_EQUALS("", errout.str());
    }

    void signConversion() {
        check("x = -4 * (unsigned)y;");
        ASSERT_EQUALS("[test.cpp:1]: (warning) Suspicious code: sign conversion of -4 in calculation because '-4' has a negative value\n", errout.str());

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

        check("long f() {\n"
              "  const long long ret = 256 * (1 << 10);\n"
              "  return ret;\n"
              "}\n", &settings);
        ASSERT_EQUALS("", errout.str());

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
    static std::string removeFloat(const std::string& msg) {
        std::string::size_type pos1 = msg.find("float (");
        std::string::size_type pos2 = msg.find(") to integer conversion");
        if (pos1 == std::string::npos || pos2 == std::string::npos || pos1 > pos2)
            return msg;
        return msg.substr(0,pos1+7) + msg.substr(pos2);
    }

    void checkFloatToIntegerOverflow() {
        check("x = (int)1E100;");
        ASSERT_EQUALS("[test.cpp:1]: (error) Undefined behaviour: float () to integer conversion overflow.\n", removeFloat(errout.str()));

        check("void f(void) {\n"
              "  return (int)1E100;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2]: (error) Undefined behaviour: float () to integer conversion overflow.\n", removeFloat(errout.str()));

        check("void f(void) {\n"
              "  return (int)-1E100;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2]: (error) Undefined behaviour: float () to integer conversion overflow.\n", removeFloat(errout.str()));

        check("void f(void) {\n"
              "  return (short)1E6;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2]: (error) Undefined behaviour: float () to integer conversion overflow.\n", removeFloat(errout.str()));

        check("void f(void) {\n"
              "  return (unsigned char)256.0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2]: (error) Undefined behaviour: float () to integer conversion overflow.\n", removeFloat(errout.str()));

        check("void f(void) {\n"
              "  return (unsigned char)255.5;\n"
              "}\n");
        ASSERT_EQUALS("", removeFloat(errout.str()));

        check("void f(void) {\n"
              "  char c = 1234.5;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (error) Undefined behaviour: float () to integer conversion overflow.\n", removeFloat(errout.str()));
    }
};

REGISTER_TEST(TestType)
