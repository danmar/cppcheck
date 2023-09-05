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


#include "checkvaarg.h"
#include "errortypes.h"
#include "settings.h"
#include "fixture.h"
#include "tokenize.h"

#include <sstream> // IWYU pragma: keep

class TestVaarg : public TestFixture {
public:
    TestVaarg() : TestFixture("TestVaarg") {}

private:
    const Settings settings = settingsBuilder().severity(Severity::warning).build();

#define check(code) check_(code, __FILE__, __LINE__)
    void check_(const char code[], const char* file, int line) {
        // Clear the error buffer..
        errout.str("");

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        ASSERT_LOC(tokenizer.tokenize(istr, "test.cpp"), file, line);

        // Check..
        runChecks<CheckVaarg>(tokenizer, this);
    }

    void run() override {
        TEST_CASE(wrongParameterTo_va_start);
        TEST_CASE(referenceAs_va_start);
        TEST_CASE(va_end_missing);
        TEST_CASE(va_list_usedBeforeStarted);
        TEST_CASE(va_start_subsequentCalls);
        TEST_CASE(unknownFunctionScope);
    }

    void wrongParameterTo_va_start() {
        check("void Format(char* szFormat, char* szBuffer, size_t nSize, ...) {\n"
              "    va_list arg_ptr;\n"
              "    va_start(arg_ptr, szFormat);\n"
              "    va_end(arg_ptr);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) 'szFormat' given to va_start() is not last named argument of the function. Did you intend to pass 'nSize'?\n", errout.str());

        check("void Format(char* szFormat, char* szBuffer, size_t nSize, ...) {\n"
              "    va_list arg_ptr;\n"
              "    va_start(arg_ptr, szBuffer);\n"
              "    va_end(arg_ptr);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) 'szBuffer' given to va_start() is not last named argument of the function. Did you intend to pass 'nSize'?\n", errout.str());

        check("void Format(char* szFormat, char* szBuffer, size_t nSize, ...) {\n"
              "    va_list arg_ptr;\n"
              "    va_start(arg_ptr, nSize);\n"
              "    va_end(arg_ptr);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("int main(int argc, char *argv[]) {\n"
              "    long(^addthem)(const char *, ...) = ^long(const char *format, ...) {\n"
              "        va_list argp;\n"
              "        va_start(argp, format);\n"
              "        c = va_arg(argp, int);\n"
              "    };\n"
              "}"); // Don't crash (#6032)
        ASSERT_EQUALS("[test.cpp:6]: (error) va_list 'argp' was opened but not closed by va_end().\n", errout.str());

        check("void Format(char* szFormat, char* szBuffer, size_t nSize, ...) {\n"
              "    va_list arg_ptr;\n"
              "    va_start(arg_ptr);\n"
              "    va_end(arg_ptr);\n"
              "}"); // Don't crash if less than expected arguments are given.
        ASSERT_EQUALS("", errout.str());

        check("void assertf_fail(const char *assertion, const char *file, int line, const char *func, const char* msg, ...) {\n"
              "    struct A {\n"
              "        A(char* buf, int size) {}\n"
              "            void printf(const char * format, ...) {\n"
              "                va_list args;\n"
              "                va_start(args, format);\n"
              "                va_end(args);\n"
              "        }\n"
              "    };\n"
              "}"); // Inner class (#6453)
        ASSERT_EQUALS("", errout.str());
    }

    void referenceAs_va_start() {
        check("void Format(char* szFormat, char (&szBuffer)[_Size], ...) {\n"
              "    va_list arg_ptr;\n"
              "    va_start(arg_ptr, szBuffer);\n"
              "    va_end(arg_ptr);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Using reference 'szBuffer' as parameter for va_start() results in undefined behaviour.\n", errout.str());

        check("void Format(char* szFormat, char (*szBuffer)[_Size], ...) {\n"
              "    va_list arg_ptr;\n"
              "    va_start(arg_ptr, szBuffer);\n"
              "    va_end(arg_ptr);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void va_end_missing() {
        check("void Format(char* szFormat, char (*szBuffer)[_Size], ...) {\n"
              "    va_list arg_ptr;\n"
              "    va_start(arg_ptr, szBuffer);\n"
              "    va_end(arg_ptr);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #6186
        check("void Format(char* szFormat, char (*szBuffer)[_Size], ...) {\n"
              "    va_list arg_ptr;\n"
              "    va_start(arg_ptr, szBuffer);\n"
              "    try {\n"
              "        throw sth;\n"
              "    } catch(...) {\n"
              "        va_end(arg_ptr);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void Format(char* szFormat, char (*szBuffer)[_Size], ...) {\n"
              "    va_list arg_ptr;\n"
              "    va_start(arg_ptr, szBuffer);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) va_list 'arg_ptr' was opened but not closed by va_end().\n", errout.str());

        check("void Format(char* szFormat, char (*szBuffer)[_Size], ...) {\n"
              "    va_list arg_ptr;\n"
              "    va_start(arg_ptr, szBuffer);\n"
              "    if(sth) return;\n"
              "    va_end(arg_ptr);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) va_list 'arg_ptr' was opened but not closed by va_end().\n", errout.str());

        // #8124
        check("void f(int n, ...)\n"
              "{\n"
              "    va_list ap;\n"
              "    va_start(ap, n);\n"
              "    std::vector<std::string> v(n);\n"
              "    std::generate_n(v.begin(), n, [&ap]()\n"
              "    {\n"
              "        return va_arg(ap, const char*);\n"
              "    });\n"
              "    va_end(ap);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f(int n, ...)\n"
              "{\n"
              "    va_list ap;\n"
              "    va_start(ap, n);\n"
              "    std::vector<std::string> v(n);\n"
              "    std::generate_n(v.begin(), n, [&ap]()\n"
              "    {\n"
              "        return va_arg(ap, const char*);\n"
              "    });\n"
              "}");
        ASSERT_EQUALS("[test.cpp:10]: (error) va_list 'ap' was opened but not closed by va_end().\n", errout.str());
    }

    void va_list_usedBeforeStarted() {
        check("void Format(char* szFormat, char (*szBuffer)[_Size], ...) {\n"
              "    va_list arg_ptr;\n"
              "    return va_arg(arg_ptr, float);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) va_list 'arg_ptr' used before va_start() was called.\n", errout.str());

        check("void Format(char* szFormat, char (*szBuffer)[_Size], ...) {\n"
              "    va_list arg_ptr;\n"
              "    foo(arg_ptr);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) va_list 'arg_ptr' used before va_start() was called.\n", errout.str());

        check("void Format(char* szFormat, char (*szBuffer)[_Size], ...) {\n"
              "    va_list arg_ptr;\n"
              "    va_copy(f, arg_ptr);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) va_list 'arg_ptr' used before va_start() was called.\n", errout.str());

        check("void Format(char* szFormat, char (*szBuffer)[_Size], ...) {\n"
              "    va_list arg_ptr;\n"
              "    va_start(arg_ptr, szBuffer);\n"
              "    va_end(arg_ptr);\n"
              "    return va_arg(arg_ptr, float);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) va_list 'arg_ptr' used before va_start() was called.\n", errout.str());

        check("void Format(char* szFormat, char (*szBuffer)[_Size], ...) {\n"
              "    va_list arg_ptr;\n"
              "    va_end(arg_ptr);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) va_list 'arg_ptr' used before va_start() was called.\n", errout.str());


        check("void Format(char* szFormat, char (*szBuffer)[_Size], ...) {\n"
              "    va_list arg_ptr;\n"
              "    va_start(arg_ptr, szBuffer);\n"
              "    va_end(arg_ptr);\n"
              "    va_start(arg_ptr, szBuffer);\n"
              "    foo(va_arg(arg_ptr, float));\n"
              "    va_end(arg_ptr);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #6066
        check("void Format(va_list v1) {\n"
              "    va_list v2;\n"
              "    va_copy(v2, v1);\n"
              "    foo(va_arg(v1, float));\n"
              "    va_end(v2);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #7527
        check("void foo(int flag1, int flag2, ...) {\n"
              "    switch (flag1) {\n"
              "    default:\n"
              "        va_list vargs;\n"
              "        va_start(vargs, flag2);\n"
              "        if (flag2) {\n"
              "            va_end(vargs);\n"
              "            break;\n"
              "        }\n"
              "        int data = va_arg(vargs, int);\n"
              "        va_end(vargs);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #7533
        check("void action_push(int type, ...) {\n"
              "    va_list args;\n"
              "    va_start(args, type);\n"
              "    switch (push_mode) {\n"
              "    case UNDO:\n"
              "        list_add(&act->node, &to_redo);\n"
              "        break;\n"
              "    case REDO:\n"
              "        list_add(&act->node, &to_undo);\n"
              "        break;\n"
              "    }\n"
              "    va_end(args);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void action_push(int type, ...) {\n"
              "    va_list args;\n"
              "    va_start(args, type);\n"
              "    switch (push_mode) {\n"
              "    case UNDO:\n"
              "        list_add(&act->node, &to_redo);\n"
              "        va_end(args);\n"
              "        break;\n"
              "    case REDO:\n"
              "        list_add(&act->node, &to_undo);\n"
              "        va_end(args);\n"
              "        break;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void action_push(int type, ...) {\n"
              "    va_list args;\n"
              "    va_start(args, type);\n"
              "    switch (push_mode) {\n"
              "    case UNDO:\n"
              "        list_add(&act->node, &to_redo);\n"
              "        break;\n"
              "    case REDO:\n"
              "        list_add(&act->node, &to_undo);\n"
              "        va_end(args);\n"
              "        break;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:13]: (error) va_list 'args' was opened but not closed by va_end().\n", errout.str());

        // #8043
        check("void redisvFormatCommand(char *format, va_list ap, bool flag) {\n"
              "    va_list _cpy;\n"
              "    va_copy(_cpy, ap);\n"
              "    if (flag)\n"
              "        goto fmt_valid;\n"
              "    va_end(_cpy);\n"
              "    goto format_err;\n"
              "fmt_valid:\n"
              "    sdscatvprintf(curarg, _format, _cpy);\n"
              "    va_end(_cpy);\n"
              "format_err:\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void va_start_subsequentCalls() {
        check("void Format(char* szFormat, char (*szBuffer)[_Size], ...) {\n"
              "    va_list arg_ptr;\n"
              "    va_start(arg_ptr, szBuffer);\n"
              "    va_start(arg_ptr, szBuffer);\n"
              "    va_end(arg_ptr);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) va_start() or va_copy() called subsequently on 'arg_ptr' without va_end() in between.\n", errout.str());

        check("void Format(char* szFormat, char (*szBuffer)[_Size], ...) {\n"
              "    va_list vl1;\n"
              "    va_start(vl1, szBuffer);\n"
              "    va_copy(vl1, vl1);\n"
              "    va_end(vl1);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) va_start() or va_copy() called subsequently on 'vl1' without va_end() in between.\n", errout.str());

        check("void Format(char* szFormat, char (*szBuffer)[_Size], ...) {\n"
              "    va_list arg_ptr;\n"
              "    va_start(arg_ptr, szBuffer);\n"
              "    va_end(arg_ptr);\n"
              "    va_start(arg_ptr, szBuffer);\n"
              "    va_end(arg_ptr);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void unknownFunctionScope() {
        check("void BG_TString::Format() {\n"
              "  BG_TChar * f;\n"
              "  va_start(args,f);\n"
              "  BG_TString result(f);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #7559
        check("void mowgli_object_message_broadcast(mowgli_object_t *self, const char *name, ...) {\n"
              "  va_list va;\n"
              "  MOWGLI_LIST_FOREACH(n, self->klass->message_handlers.head) {\n"
              "    if (!strcasecmp(sig2->name, name))\n"
              "      break;\n"
              "  }\n"
              "  va_start(va, name);\n"
              "  va_end(va);\n"
              "}");
    }
};

REGISTER_TEST(TestVaarg)
