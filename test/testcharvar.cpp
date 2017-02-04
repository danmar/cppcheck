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

#include "tokenize.h"
#include "checkother.h"
#include "testsuite.h"


class TestCharVar : public TestFixture {
public:
    TestCharVar() : TestFixture("TestCharVar") {
    }

private:
    Settings settings;

    void run() {
        settings.platform(Settings::Unspecified);
        settings.addEnabled("warning");
        settings.addEnabled("portability");

        TEST_CASE(array_index_1);
        TEST_CASE(array_index_2);
        TEST_CASE(bitop);
    }

    void check(const char code[]) {
        // Clear the error buffer..
        errout.str("");

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
        ASSERT_EQUALS("[test.cpp:5]: (portability) 'char' type used as array index.\n", errout.str());

        check("int buf[256];\n"
              "void foo()\n"
              "{\n"
              "    char ch = 0;\n"
              "    buf[ch] = 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("int buf[256];\n"
              "void foo()\n"
              "{\n"
              "    signed char ch = 0x80;\n"
              "    buf[ch] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (warning) Signed 'char' type used as array index.\n", errout.str());

        check("int buf[256];\n"
              "void foo()\n"
              "{\n"
              "    signed char ch = 0;\n"
              "    buf[ch] = 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("int buf[256];\n"
              "void foo()\n"
              "{\n"
              "    char ch = 0x80;\n"
              "    buf[ch] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (portability) 'char' type used as array index.\n", errout.str());

        check("int buf[256];\n"
              "void foo(signed char ch)\n"
              "{\n"
              "    buf[ch] = 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("int buf[256];\n"
              "void foo(char ch)\n"
              "{\n"
              "    buf[ch] = 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo(char* buf)\n"
              "{\n"
              "    char ch = 0x80;"
              "    buf[ch] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (portability) 'char' type used as array index.\n", errout.str());

        check("void foo(char* buf)\n"
              "{\n"
              "    char ch = 0;"
              "    buf[ch] = 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo(char* buf)\n"
              "{\n"
              "    buf['A'] = 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo(char* buf, char ch)\n"
              "{\n"
              "    buf[ch] = 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("int flags[256];\n"
              "void foo(const char* str)\n"
              "{\n"
              "    flags[*str] = 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("int flags[256];\n"
              "void foo(const char* str)\n"
              "{\n"
              "    flags[(unsigned char)*str] = 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

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

    void bitop() {
        check("void foo(int *result) {\n"
              "    signed char ch = -1;\n"
              "    *result = a | ch;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) When using 'char' variables in bit operations, sign extension can generate unexpected results.\n", errout.str());

        check("void foo(int *result) {\n"
              "    unsigned char ch = -1;\n"
              "    *result = a | ch;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo(char *result) {\n"
              "    signed char ch = -1;\n"
              "    *result = a | ch;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // 0x03 & ..
        check("void foo(int *result) {\n"
              "    signed char ch = -1;\n"
              "    *result = 0x03 | ch;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) When using 'char' variables in bit operations, sign extension can generate unexpected results.\n", errout.str());

        check("void foo(int *result) {\n"
              "    signed char ch = -1;\n"
              "    *result = 0x03 & ch;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }
};

REGISTER_TEST(TestCharVar)
