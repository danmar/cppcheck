/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2015 Daniel Marjamäki and Cppcheck team.
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

#include "tokenize.h"
#include "checkother.h"
#include "testsuite.h"


class TestCharVar : public TestFixture {
public:
    TestCharVar() : TestFixture("TestCharVar") {
    }

private:


    void run() {
        TEST_CASE(array_index_1);
        TEST_CASE(array_index_2);
        TEST_CASE(array_index_3);
        TEST_CASE(bitop1);
        TEST_CASE(bitop2);
        TEST_CASE(bitop3);
        TEST_CASE(bitop4); // (long)&c
        TEST_CASE(return1);
        TEST_CASE(assignChar);
        TEST_CASE(and03);
        TEST_CASE(pointer);
    }

    void check(const char code[]) {
        // Clear the error buffer..
        errout.str("");

        Settings settings;
        settings.addEnabled("warning");

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        // Check char variable usage..
        CheckOther checkOther(&tokenizer, &settings, this);
        checkOther.checkCharVariable();
    }

    void array_index_1() {
        check("int buf[256];\n"
              "void foo()\n"
              "{\n"
              "    unsigned char ch = 0x80;\n"
              "    buf[ch] = 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("int buf[256];\n"
              "void foo()\n"
              "{\n"
              "    char ch = 0x80;\n"
              "    buf[ch] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (warning) Signed 'char' type used as array index.\n", errout.str());

        check("int buf[256];\n"
              "void foo()\n"
              "{\n"
              "    signed char ch = 0x80;\n"
              "    buf[ch] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (warning) Signed 'char' type used as array index.\n", errout.str());

        check("int buf[256];\n"
              "void foo(char ch)\n"
              "{\n"
              "    buf[ch] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (warning) Signed 'char' type used as array index.\n", errout.str());

        check("void foo(const char str[])\n"
              "{\n"
              "    map[str] = 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void array_index_2() {
        // #3282 - False positive
        check("void foo(char i);\n"
              "void bar(int i) {\n"
              "    const char *s = \"abcde\";\n"
              "    foo(s[i]);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void array_index_3() {
        // only write error message when array is more than
        // 0x80 elements in size. Otherwise the full valid
        // range is accessible with a char.

        check("char buf[0x81];\n"
              "void bar(char c) {\n"
              "    buf[c] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Signed 'char' type used as array index.\n", errout.str());

        check("char buf[0x80];\n"
              "void bar(char c) {\n"
              "    buf[c] = 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void bar(char c) {\n"
              "    buf[c] = 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void bitop1() {
        check("void foo()\n"
              "{\n"
              "    int result = 0;\n"
              "    char ch;\n"
              "    result = a | ch;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (warning) When using 'char' variables in bit operations, sign extension can generate unexpected results.\n", errout.str());
    }

    void bitop2() {
        check("void foo()\n"
              "{\n"
              "    char ch;\n"
              "    func(&ch);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void bitop3() {
        check("void f(int& i, char& c) {\n"
              "    i &= c;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) When using 'char' variables in bit operations, sign extension can generate unexpected results.\n", errout.str());
    }

    void bitop4() {
        check("long f(char c) {\n"
              "  long a;\n"
              "  a = (long)&c;\n"
              "  return a;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void return1() {
        check("void foo()\n"
              "{\n"
              "    char c;\n"
              "    return &c;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }


    void assignChar() {
        check("void foo()\n"
              "{\n"
              "    char c;\n"
              "    c = c & 0x123;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void and03() {
        check("void foo()\n"
              "{\n"
              "    char c;\n"
              "    int i = c & 0x03;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void pointer() {
        // ticket #2866
        check("void f(char *p) {\n"
              "    int ret = 0;\n"
              "    ret |= *p;\n"
              "    return ret;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) When using 'char' variables in bit operations, sign extension can generate unexpected results.\n", errout.str());

        // fixed code
        check("void f(char *p) {\n"
              "    int ret = 0;\n"
              "    ret |= (unsigned char)*p;\n"
              "    return ret;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #3872 - false positive
        check("int f(int *p) {\n"
              "    int ret = a();\n"
              "    ret |= *p;\n"
              "    return ret;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #3878 - false positive
        check("int f(unsigned char *p) {\n"
              "    int ret = a();\n"
              "    ret |= *p;\n"
              "    return ret;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }
};

REGISTER_TEST(TestCharVar)
