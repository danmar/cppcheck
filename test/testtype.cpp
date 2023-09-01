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

#include "checktype.h"
#include "errortypes.h"
#include "platform.h"
#include "settings.h"
#include "standards.h"
#include "fixture.h"
#include "tokenize.h"

#include <sstream> // IWYU pragma: keep
#include <string>

class TestType : public TestFixture {
public:
    TestType() : TestFixture("TestType") {}

private:


    void run() override {
        TEST_CASE(checkTooBigShift_Unix32);
        TEST_CASE(checkIntegerOverflow);
        TEST_CASE(signConversion);
        TEST_CASE(longCastAssign);
        TEST_CASE(longCastReturn);
        TEST_CASE(checkFloatToIntegerOverflow);
    }

#define check(...) check_(__FILE__, __LINE__, __VA_ARGS__)
    void check_(const char* file, int line, const char code[], const Settings& settings, const char filename[] = "test.cpp", Standards::cppstd_t standard = Standards::cppstd_t::CPP11) {
        // Clear the error buffer..
        errout.str("");

        const Settings settings1 = settingsBuilder(settings).severity(Severity::warning).severity(Severity::portability).cpp(standard).build();

        // Tokenize..
        Tokenizer tokenizer(&settings1, this);
        std::istringstream istr(code);
        ASSERT_LOC(tokenizer.tokenize(istr, filename), file, line);

        // Check..
        runChecks<CheckType>(tokenizer, this);
    }

    void checkTooBigShift_Unix32() {
        const Settings settings0;
        const Settings settings = settingsBuilder().platform(cppcheck::Platform::Type::Unix32).build();

        // unsigned types getting promoted to int sizeof(int) = 4 bytes
        // and unsigned types having already a size of 4 bytes
        {
            const std::string types[] = {"unsigned char", /*[unsigned]*/ "char", "bool", "unsigned short", "unsigned int", "unsigned long"};
            for (const std::string& type : types) {
                check((type + " f(" + type +" x) { return x << 31; }").c_str(), settings);
                ASSERT_EQUALS("", errout.str());
                check((type + " f(" + type +" x) { return x << 33; }").c_str(), settings);
                ASSERT_EQUALS("[test.cpp:1]: (error) Shifting 32-bit value by 33 bits is undefined behaviour\n", errout.str());
                check((type + " f(int x) { return (x = (" + type + ")x << 32); }").c_str(), settings);
                ASSERT_EQUALS("[test.cpp:1]: (error) Shifting 32-bit value by 32 bits is undefined behaviour\n", errout.str());
                check((type + " foo(" + type + " x) { return x << 31; }").c_str(), settings);
                ASSERT_EQUALS("", errout.str());
            }
        }
        // signed types getting promoted to int sizeof(int) = 4 bytes
        // and signed types having already a size of 4 bytes
        {
            const std::string types[] = {"signed char", "signed short", /*[signed]*/ "short", "wchar_t", /*[signed]*/ "int", "signed int", /*[signed]*/ "long", "signed long"};
            for (const std::string& type : types) {
                // c++11
                check((type + " f(" + type +" x) { return x << 33; }").c_str(), settings);
                ASSERT_EQUALS("[test.cpp:1]: (error) Shifting 32-bit value by 33 bits is undefined behaviour\n", errout.str());
                check((type + " f(int x) { return (x = (" + type + ")x << 32); }").c_str(), settings);
                ASSERT_EQUALS("[test.cpp:1]: (error) Shifting 32-bit value by 32 bits is undefined behaviour\n", errout.str());
                check((type + " foo(" + type + " x) { return x << 31; }").c_str(), settings);
                ASSERT_EQUALS("[test.cpp:1]: (error) Shifting signed 32-bit value by 31 bits is undefined behaviour\n", errout.str());
                check((type + " foo(" + type + " x) { return x << 30; }").c_str(), settings);
                ASSERT_EQUALS("", errout.str());

                // c++14
                check((type + " foo(" + type + " x) { return x << 31; }").c_str(), settings, "test.cpp", Standards::cppstd_t::CPP14);
                ASSERT_EQUALS("[test.cpp:1]: (portability) Shifting signed 32-bit value by 31 bits is implementation-defined behaviour\n", errout.str());
                check((type + " f(int x) { return (x = (" + type + ")x << 32); }").c_str(), settings, "test.cpp", Standards::cppstd_t::CPP14);
                ASSERT_EQUALS("[test.cpp:1]: (error) Shifting 32-bit value by 32 bits is undefined behaviour\n", errout.str());
            }
        }
        // 64 bit width types
        {
            // unsigned long long
            check("unsigned long long foo(unsigned long long x) { return x << 64; }",settings);
            ASSERT_EQUALS("[test.cpp:1]: (error) Shifting 64-bit value by 64 bits is undefined behaviour\n", errout.str());
            check("unsigned long long f(int x) { return (x = (unsigned long long)x << 64); }",settings);
            ASSERT_EQUALS("[test.cpp:1]: (error) Shifting 64-bit value by 64 bits is undefined behaviour\n", errout.str());
            check("unsigned long long f(unsigned long long x) { return x << 63; }",settings);
            ASSERT_EQUALS("", errout.str());
            // [signed] long long
            check("long long foo(long long x) { return x << 64; }",settings);
            ASSERT_EQUALS("[test.cpp:1]: (error) Shifting 64-bit value by 64 bits is undefined behaviour\n", errout.str());
            check("long long f(int x) { return (x = (long long)x << 64); }",settings);
            ASSERT_EQUALS("[test.cpp:1]: (error) Shifting 64-bit value by 64 bits is undefined behaviour\n", errout.str());
            check("long long f(long long x) { return x << 63; }",settings);
            ASSERT_EQUALS("[test.cpp:1]: (error) Shifting signed 64-bit value by 63 bits is undefined behaviour\n", errout.str());
            check("long long f(long long x) { return x << 62; }",settings);
            ASSERT_EQUALS("", errout.str());
            // signed long long
            check("signed long long foo(signed long long x) { return x << 64; }",settings);
            ASSERT_EQUALS("[test.cpp:1]: (error) Shifting 64-bit value by 64 bits is undefined behaviour\n", errout.str());
            check("signed long long f(long long x) { return (x = (signed long long)x << 64); }",settings);
            ASSERT_EQUALS("[test.cpp:1]: (error) Shifting 64-bit value by 64 bits is undefined behaviour\n", errout.str());
            check("signed long long f(signed long long x) { return x << 63; }",settings);
            ASSERT_EQUALS("[test.cpp:1]: (error) Shifting signed 64-bit value by 63 bits is undefined behaviour\n", errout.str());
            check("signed long long f(signed long long x) { return x << 62; }",settings);
            ASSERT_EQUALS("", errout.str());

            // c++14
            check("signed long long foo(signed long long x) { return x << 64; }",settings, "test.cpp", Standards::cppstd_t::CPP14);
            ASSERT_EQUALS("[test.cpp:1]: (error) Shifting 64-bit value by 64 bits is undefined behaviour\n", errout.str());
            check("signed long long f(long long x) { return (x = (signed long long)x << 64); }",settings, "test.cpp", Standards::cppstd_t::CPP14);
            ASSERT_EQUALS("[test.cpp:1]: (error) Shifting 64-bit value by 64 bits is undefined behaviour\n", errout.str());
            check("signed long long f(signed long long x) { return x << 63; }",settings, "test.cpp", Standards::cppstd_t::CPP14);
            ASSERT_EQUALS("[test.cpp:1]: (portability) Shifting signed 64-bit value by 63 bits is implementation-defined behaviour\n", errout.str());
            check("signed long long f(signed long long x) { return x << 62; }",settings);
            ASSERT_EQUALS("", errout.str());
        }

        check("void f() { int x; x = 1 >> 64; }", settings);
        ASSERT_EQUALS("[test.cpp:1]: (error) Shifting 32-bit value by 64 bits is undefined behaviour\n", errout.str());

        check("void foo() {\n"
              "  QList<int> someList;\n"
              "  someList << 300;\n"
              "}", settings);
        ASSERT_EQUALS("", errout.str());

        // Ticket #6793
        check("template<unsigned int I> int foo(unsigned int x) { return x << I; }\n"
              "const unsigned int f = foo<31>(0);\n"
              "const unsigned int g = foo<100>(0);\n"
              "template<unsigned int I> int hoo(unsigned int x) { return x << 32; }\n"
              "const unsigned int h = hoo<100>(0);", settings);
        ASSERT_EQUALS("[test.cpp:4]: (error) Shifting 32-bit value by 32 bits is undefined behaviour\n"
                      "[test.cpp:1]: (error) Shifting 32-bit value by 100 bits is undefined behaviour\n", errout.str());

        // #7266: C++, shift in macro
        check("void f(unsigned int x) {\n"
              "    UINFO(x << 1234);\n"
              "}", settings0);
        ASSERT_EQUALS("", errout.str());

        // #8640
        check("int f (void)\n"
              "{\n"
              "    constexpr const int a = 1;\n"
              "    constexpr const int shift[1] = {32};\n"
              "    constexpr const int ret = a << shift[0];\n" // shift too many bits
              "    return ret;\n"
              "}", settings0);
        ASSERT_EQUALS("[test.cpp:5]: (error) Shifting 32-bit value by 32 bits is undefined behaviour\n"
                      "[test.cpp:5]: (error) Signed integer overflow for expression 'a<<shift[0]'.\n", errout.str());

        // #8885
        check("int f(int k, int rm) {\n"
              "  if (k == 32)\n"
              "    return 0;\n"
              "  if (k > 32)\n"
              "    return 0;\n"
              "  return rm>> k;\n"
              "}", settings0);
        ASSERT_EQUALS(
            "[test.cpp:4] -> [test.cpp:6]: (warning) Shifting signed 32-bit value by 31 bits is undefined behaviour. See condition at line 4.\n",
            errout.str());

        check("int f(int k, int rm) {\n"
              "  if (k == 0 || k == 32)\n"
              "    return 0;\n"
              "  else if (k > 32)\n"
              "    return 0;\n"
              "  else\n"
              "    return rm>> k;\n"
              "}", settings0);
        ASSERT_EQUALS(
            "[test.cpp:4] -> [test.cpp:7]: (warning) Shifting signed 32-bit value by 31 bits is undefined behaviour. See condition at line 4.\n",
            errout.str());

        check("int f(int k, int rm) {\n"
              "  if (k == 0 || k == 32 || k == 31)\n"
              "    return 0;\n"
              "  else if (k > 32)\n"
              "    return 0;\n"
              "  else\n"
              "    return rm>> k;\n"
              "}", settings0);
        ASSERT_EQUALS("", errout.str());

        check("static long long f(int x, long long y) {\n"
              "    if (x >= 64)\n"
              "        return 0;\n"
              "    return -(y << (x-1));\n"
              "}", settings0);
        ASSERT_EQUALS("", errout.str());

        check("bool f() {\n"
              "    std::ofstream outfile;\n"
              "    outfile << vec_points[0](0) << static_cast<int>(d) << ' ';\n"
              "}", settings0);
        ASSERT_EQUALS("", errout.str());

        check("void f(unsigned b, int len, unsigned char rem) {\n" // #10773
              "    int bits = 0;\n"
              "    while (len > 8) {\n"
              "        b = b >> rem;\n"
              "        bits += 8 - rem;\n"
              "        if (bits == 512)\n"
              "            len -= 8;\n"
              "    }\n"
              "}\n", settings0);
        ASSERT_EQUALS("", errout.str());
    }

    void checkIntegerOverflow() {
        const Settings settings = settingsBuilder().severity(Severity::warning).platform(cppcheck::Platform::Type::Unix32).build();

        check("x = (int)0x10000 * (int)0x10000;", settings);
        ASSERT_EQUALS("[test.cpp:1]: (error) Signed integer overflow for expression '(int)0x10000*(int)0x10000'.\n", errout.str());

        check("x = (long)0x10000 * (long)0x10000;", settings);
        ASSERT_EQUALS("[test.cpp:1]: (error) Signed integer overflow for expression '(long)0x10000*(long)0x10000'.\n", errout.str());

        check("void foo() {\n"
              "    int intmax = 0x7fffffff;\n"
              "    return intmax + 1;\n"
              "}",settings);
        ASSERT_EQUALS("[test.cpp:3]: (error) Signed integer overflow for expression 'intmax+1'.\n", errout.str());

        check("void foo() {\n"
              "    int intmax = 0x7fffffff;\n"
              "    return intmax - 1;\n"
              "}",settings);
        ASSERT_EQUALS("", errout.str());

        check("int foo(signed int x) {\n"
              "   if (x==123456) {}\n"
              "   return x * x;\n"
              "}",settings);
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (warning) Either the condition 'x==123456' is redundant or there is signed integer overflow for expression 'x*x'.\n", errout.str());

        check("int foo(signed int x) {\n"
              "   if (x==123456) {}\n"
              "   return -123456 * x;\n"
              "}",settings);
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (warning) Either the condition 'x==123456' is redundant or there is signed integer overflow for expression '-123456*x'.\n", errout.str());

        check("int foo(signed int x) {\n"
              "   if (x==123456) {}\n"
              "   return 123456U * x;\n"
              "}",settings);
        ASSERT_EQUALS("", errout.str());
    }

    void signConversion() {
        const Settings settings0;
        const Settings settings = settingsBuilder().platform(cppcheck::Platform::Type::Unix64).build();
        check("x = -4 * (unsigned)y;", settings0);
        ASSERT_EQUALS("[test.cpp:1]: (warning) Expression '-4' has a negative value. That is converted to an unsigned value and used in an unsigned calculation.\n", errout.str());

        check("x = (unsigned)y * -4;", settings0);
        ASSERT_EQUALS("[test.cpp:1]: (warning) Expression '-4' has a negative value. That is converted to an unsigned value and used in an unsigned calculation.\n", errout.str());

        check("unsigned int dostuff(int x) {\n" // x is signed
              "  if (x==0) {}\n"
              "  return (x-1)*sizeof(int);\n"
              "}", settings);
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:3]: (warning) Expression 'x-1' can have a negative value. That is converted to an unsigned value and used in an unsigned calculation.\n", errout.str());

        check("unsigned int f1(signed int x, unsigned int y) {" // x is signed
              "  return x * y;\n"
              "}\n"
              "void f2() { f1(-4,4); }", settings0);
        ASSERT_EQUALS(
            "[test.cpp:1]: (warning) Expression 'x' can have a negative value. That is converted to an unsigned value and used in an unsigned calculation.\n",
            errout.str());

        check("unsigned int f1(int x) {"
              "  return x * 5U;\n"
              "}\n"
              "void f2() { f1(-4); }", settings0);
        ASSERT_EQUALS(
            "[test.cpp:1]: (warning) Expression 'x' can have a negative value. That is converted to an unsigned value and used in an unsigned calculation.\n",
            errout.str());

        check("unsigned int f1(int x) {" // #6168: FP for inner calculation
              "  return 5U * (1234 - x);\n" // <- signed subtraction, x is not sign converted
              "}\n"
              "void f2() { f1(-4); }", settings0);
        ASSERT_EQUALS("", errout.str());

        // Don't warn for + and -
        check("void f1(int x) {"
              "  a = x + 5U;\n"
              "}\n"
              "void f2() { f1(-4); }", settings0);
        ASSERT_EQUALS("", errout.str());

        check("size_t foo(size_t x) {\n"
              " return -2 * x;\n"
              "}", settings0);
        ASSERT_EQUALS("[test.cpp:2]: (warning) Expression '-2' has a negative value. That is converted to an unsigned value and used in an unsigned calculation.\n", errout.str());
    }

    void longCastAssign() {
        const Settings settings = settingsBuilder().severity(Severity::style).platform(cppcheck::Platform::Type::Unix64).build();
        const Settings settingsWin = settingsBuilder().severity(Severity::style).platform(cppcheck::Platform::Type::Win64).build();

        const char code[] = "long f(int x, int y) {\n"
                            "  const long ret = x * y;\n"
                            "  return ret;\n"
                            "}\n";
        check(code, settings);
        ASSERT_EQUALS("[test.cpp:2]: (style) int result is assigned to long variable. If the variable is long to avoid loss of information, then you have loss of information.\n", errout.str());
        check(code, settingsWin);
        ASSERT_EQUALS("", errout.str());

        check("long f(int x, int y) {\n"
              "  long ret = x * y;\n"
              "  return ret;\n"
              "}\n", settings);
        ASSERT_EQUALS("[test.cpp:2]: (style) int result is assigned to long variable. If the variable is long to avoid loss of information, then you have loss of information.\n", errout.str());

        check("long f() {\n"
              "  const long long ret = 256 * (1 << 10);\n"
              "  return ret;\n"
              "}\n", settings);
        ASSERT_EQUALS("", errout.str());

        // typedef
        check("long f(int x, int y) {\n"
              "  const size_t ret = x * y;\n"
              "  return ret;\n"
              "}\n", settings);
        ASSERT_EQUALS("", errout.str());

        // astIsIntResult
        check("long f(int x, int y) {\n"
              "  const long ret = (long)x * y;\n"
              "  return ret;\n"
              "}\n", settings);
        ASSERT_EQUALS("", errout.str());

        check("double g(float f) {\n"
              "    return f * f;\n"
              "}\n", settings);
        ASSERT_EQUALS("[test.cpp:2]: (style) float result is returned as double value. If the return value is double to avoid loss of information, then you have loss of information.\n",
                      errout.str());

        check("void f(int* p) {\n" // #11862
              "    long long j = *(p++);\n"
              "}\n", settings);
        ASSERT_EQUALS("", errout.str());
    }

    void longCastReturn() {
        const Settings settings = settingsBuilder().severity(Severity::style).platform(cppcheck::Platform::Type::Unix64).build();
        const Settings settingsWin = settingsBuilder().severity(Severity::style).platform(cppcheck::Platform::Type::Win64).build();

        const char code[] = "long f(int x, int y) {\n"
                            "  return x * y;\n"
                            "}\n";
        check(code, settings);
        ASSERT_EQUALS("[test.cpp:2]: (style) int result is returned as long value. If the return value is long to avoid loss of information, then you have loss of information.\n", errout.str());
        check(code, settingsWin);
        ASSERT_EQUALS("", errout.str());

        const char code2[] = "long long f(int x, int y) {\n"
                             "  return x * y;\n"
                             "}\n";
        check(code2, settings);
        ASSERT_EQUALS("[test.cpp:2]: (style) int result is returned as long long value. If the return value is long long to avoid loss of information, then you have loss of information.\n", errout.str());
        check(code2, settingsWin);
        ASSERT_EQUALS("[test.cpp:2]: (style) int result is returned as long long value. If the return value is long long to avoid loss of information, then you have loss of information.\n", errout.str());

        // typedef
        check("size_t f(int x, int y) {\n"
              "  return x * y;\n"
              "}\n", settings);
        ASSERT_EQUALS("[test.cpp:2]: (style) int result is returned as long value. If the return value is long to avoid loss of information, then you have loss of information.\n", errout.str());
    }

    // This function ensure that test works with different compilers. Floats can
    // be stringified differently.
    static std::string removeFloat(const std::string& msg) {
        const std::string::size_type pos1 = msg.find("float (");
        const std::string::size_type pos2 = msg.find(") to integer conversion");
        if (pos1 == std::string::npos || pos2 == std::string::npos || pos1 > pos2)
            return msg;
        return msg.substr(0,pos1+7) + msg.substr(pos2);
    }

    void checkFloatToIntegerOverflow() {
        const Settings settings;
        check("x = (int)1E100;", settings);
        ASSERT_EQUALS("[test.cpp:1]: (error) Undefined behaviour: float () to integer conversion overflow.\n", removeFloat(errout.str()));

        check("void f(void) {\n"
              "  return (int)1E100;\n"
              "}", settings);
        ASSERT_EQUALS("[test.cpp:2]: (error) Undefined behaviour: float () to integer conversion overflow.\n", removeFloat(errout.str()));

        check("void f(void) {\n"
              "  return (int)-1E100;\n"
              "}", settings);
        ASSERT_EQUALS("[test.cpp:2]: (error) Undefined behaviour: float () to integer conversion overflow.\n", removeFloat(errout.str()));

        check("void f(void) {\n"
              "  return (short)1E6;\n"
              "}", settings);
        ASSERT_EQUALS("[test.cpp:2]: (error) Undefined behaviour: float () to integer conversion overflow.\n", removeFloat(errout.str()));

        check("void f(void) {\n"
              "  return (unsigned char)256.0;\n"
              "}", settings);
        ASSERT_EQUALS("[test.cpp:2]: (error) Undefined behaviour: float () to integer conversion overflow.\n", removeFloat(errout.str()));

        check("void f(void) {\n"
              "  return (unsigned char)255.5;\n"
              "}", settings);
        ASSERT_EQUALS("", removeFloat(errout.str()));

        check("void f(void) {\n"
              "  char c = 1234.5;\n"
              "}", settings);
        ASSERT_EQUALS("[test.cpp:2]: (error) Undefined behaviour: float () to integer conversion overflow.\n", removeFloat(errout.str()));

        check("char f(void) {\n"
              "  return 1234.5;\n"
              "}", settings);
        ASSERT_EQUALS("[test.cpp:2]: (error) Undefined behaviour: float () to integer conversion overflow.\n", removeFloat(errout.str()));
    }
};

REGISTER_TEST(TestType)
