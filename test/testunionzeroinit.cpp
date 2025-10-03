/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2025 Cppcheck team.
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

#include "checkunionzeroinit.h"
#include "errortypes.h"
#include "fixture.h"
#include "helpers.h"
#include "settings.h"

static std::string expectedErrorMessage(int lno, int cno, const std::string &varName, const std::string &largestMemberName)
{
    std::stringstream ss;
    ss << "[test.cpp:" << lno << ":" << cno << "]: (portability) ";
    ss << "Zero initializing union '" << varName << "' ";
    ss << "does not guarantee its complete storage to be zero initialized as its largest member is not declared as the first member. ";
    ss << "Consider making " << largestMemberName << " the first member or favor memset(). [UnionZeroInit]";
    ss << std::endl;
    return ss.str();
}

class TestUnionZeroInit : public TestFixture {
public:
    TestUnionZeroInit() : TestFixture("TestUnionZeroInit") {}

private:
    const Settings mSettings = settingsBuilder().severity(Severity::portability).library("std.cfg").build();
    std::string mMessage;

    void run() override {
        mNewTemplate = true;
        TEST_CASE(basic);
        TEST_CASE(arrayMember);
        TEST_CASE(structMember);
        TEST_CASE(unknownType);
        TEST_CASE(bitfields);
    }

#define checkUnionZeroInit(...) checkUnionZeroInit_(__FILE__, __LINE__, __VA_ARGS__)
    template<size_t size>
    void checkUnionZeroInit_(const char* file, int line, const char (&code)[size], bool cpp = true) {
        // Tokenize..
        SimpleTokenizer tokenizer(mSettings, *this, cpp);
        ASSERT_LOC(tokenizer.tokenize(code), file, line);

        CheckUnionZeroInit(&tokenizer, &mSettings, this).check();

        mMessage = CheckUnionZeroInit::generateTestMessage(tokenizer, mSettings);
    }

    void basic() {
        checkUnionZeroInit(
            "union bad_union_0 {\n"
            "  char c;\n"
            "  long long i64;\n"
            "  void *p;\n"
            "};\n"
            "\n"
            "typedef union {\n"
            "  char c;\n"
            "  int i;\n"
            "} bad_union_1;\n"
            "\n"
            "extern void e(union bad_union_0 *);\n"
            "\n"
            "void\n"
            "foo(void)\n"
            "{\n"
            "  union { int i; char c; } good0 = {0};\n"
            "  union { int i; char c; } good1 = {};\n"
            "\n"
            "  union { char c; int i; } bad0 = {0};\n"
            "  union bad_union_0 bad1 = {0};\n"
            "  e(&bad1);\n"
            "  bad_union_1 bad2 = {0};\n"
            "}");
        const std::string exp = expectedErrorMessage(20, 28, "bad0", "i") +
                                expectedErrorMessage(21, 21, "bad1", "i64") +
                                expectedErrorMessage(23, 15, "bad2", "i");
        ASSERT_EQUALS_MSG(exp, errout_str(), mMessage);
    }

    void arrayMember() {
        checkUnionZeroInit(
            "void foo(void) {\n"
            " union { int c; char s8[2]; } u = {0};\n"
            "}");
        ASSERT_EQUALS_MSG("", errout_str(), mMessage);
    }

    void structMember() {
        checkUnionZeroInit(
            "void foo(void) {\n"
            "  union {\n"
            "    int c;\n"
            "     struct {\n"
            "       char x;\n"
            "       struct {\n"
            "         char y;\n"
            "       } s1;\n"
            "    } s0;\n"
            "  } u = {0};\n"
            "}");
        ASSERT_EQUALS_MSG("", errout_str(), mMessage);
    }

    void unknownType() {
        checkUnionZeroInit(
            "union u {\n"
            "  Unknown x;\n"
            "}");
        ASSERT_EQUALS_MSG("", errout_str(), mMessage);
    }

    void bitfields() {
        checkUnionZeroInit(
            "typedef union Evex {\n"
            "  int u32;\n"
            "  struct {\n"
            "    char mmm:3,\n"
            "         b4:1,\n"
            "         r4:1,\n"
            "         b3:1,\n"
            "         x3:1,\n"
            "         r3:1;\n"
            "  } extended;\n"
            "} Evex;\n"
            "\n"
            "void foo(void) {\n"
            "  Evex evex = {0};\n"
            "}");
        ASSERT_EQUALS_MSG("", errout_str(), mMessage);
    }
};
REGISTER_TEST(TestUnionZeroInit)
