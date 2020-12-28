/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2020 Cppcheck team.
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

#include "config.h"
#include "exprengine.h"
#include "settings.h"
#include "tokenize.h"
#include "testsuite.h"

class TestBughuntingChecks : public TestFixture {
public:
    TestBughuntingChecks() : TestFixture("TestBughuntingChecks") {
        settings.platform(cppcheck::Platform::Unix64);
    }

private:
    Settings settings;

    void run() OVERRIDE {
#ifdef USE_Z3
        settings.inconclusive = true;
        LOAD_LIB_2(settings.library, "std.cfg");
        TEST_CASE(checkAssignment);
        TEST_CASE(arrayIndexOutOfBounds1);
        TEST_CASE(arrayIndexOutOfBounds2);
        TEST_CASE(arrayIndexOutOfBounds3);
        TEST_CASE(arrayIndexOutOfBounds4);
        TEST_CASE(arrayIndexOutOfBounds5);
        TEST_CASE(arrayIndexOutOfBoundsDim1);
        TEST_CASE(bufferOverflowMemCmp1);
        TEST_CASE(bufferOverflowMemCmp2);
        TEST_CASE(bufferOverflowStrcpy1);
        TEST_CASE(bufferOverflowStrcpy2);

        TEST_CASE(divisionByZeroNoReturn);

        TEST_CASE(uninit);
        TEST_CASE(uninit_array);
        TEST_CASE(uninit_function_par);
        TEST_CASE(uninit_malloc);
        TEST_CASE(uninit_struct);
        TEST_CASE(uninit_bailout);
        TEST_CASE(uninit_fp_smartptr);
        TEST_CASE(uninit_fp_struct);
        TEST_CASE(uninit_fp_struct_member_init_2);
        TEST_CASE(uninit_fp_template_var);
        TEST_CASE(ctu);
#endif
    }

    void check(const char code[]) {
        settings.bugHunting = settings.library.bugHunting = true;
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");
        errout.str("");
        ExprEngine::runChecks(this, &tokenizer, &settings);
    }

    void checkAssignment() {
        check("void foo(int any) { __cppcheck_low__(0) int x; x = any; }");
        ASSERT_EQUALS("[test.cpp:1]: (error) There is assignment, cannot determine that value is greater or equal with 0\n", errout.str());

        check("struct S { __cppcheck_low__(0) int x; };\n"
              "void foo(S *s, int any) { s->x = any; }");
        ASSERT_EQUALS("[test.cpp:2]: (error) There is assignment, cannot determine that value is greater or equal with 0\n", errout.str());
    }

    void arrayIndexOutOfBounds1() {
        check("void foo(int x) {\n"
              "  int p[8];"
              "  p[x] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (error) Array index out of bounds, cannot determine that x is less than 8\n"
                      "[test.cpp:2]: (error) Array index out of bounds, cannot determine that x is not negative\n",
                      errout.str());
    }

    void arrayIndexOutOfBounds2() { // loop
        check("void foo(int n) {\n"
              "  int p[8];\n"
              "  for (int i = 0; i < n; i++)\n"
              "    p[i] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Array index out of bounds, cannot determine that i is less than 8\n"
                      "[test.cpp:4]: (error) Array index out of bounds, cannot determine that i is not negative\n",
                      errout.str());

        // .. with unknown expression
        check("void foo(int n) {\n"
              "  int p[8];\n"
              "  crx_data_header_t *d =\n"
              "    &libraw_internal_data.unpacker_data.crx_header[framei];\n"
              "  for (int i = 0; i < n; i++)\n"
              "    p[i] = 0;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:6]: (error) Array index out of bounds, cannot determine that i is less than 8\n"
                      "[test.cpp:6]: (error) Array index out of bounds, cannot determine that i is not negative\n",
                      errout.str());
    }

    void arrayIndexOutOfBounds3() { // struct
        check("struct S { int x; };\n"
              "void foo(short i) {\n"
              "  S s[8];\n"
              "  if (s[i].x == 0) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Array index out of bounds, cannot determine that i is less than 8\n"
                      "[test.cpp:4]: (error) Array index out of bounds, cannot determine that i is not negative\n"
                      "[test.cpp:4]: (error) Cannot determine that 's[i]' is initialized\n",
                      errout.str());
    }

    void arrayIndexOutOfBounds4() { // ensure there are warnings for bailout value
        check("void foo(short i) {\n"
              "    int buf[8];\n"
              "\n"
              "    data *d = x;\n"
              "    switch (d->layout) { case 0: break; }\n"
              "\n"
              "    if (buf[i] > 0) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7]: (error) Array index out of bounds, cannot determine that i is less than 8\n"
                      "[test.cpp:7]: (error) Array index out of bounds, cannot determine that i is not negative\n"
                      "[test.cpp:7]: (error) Cannot determine that 'buf[i]' is initialized\n",
                      errout.str());
    }

    void arrayIndexOutOfBounds5() {
        check("struct {\n"
              "    struct { int z; } y;\n"
              "} x;\n"
              "\n"
              "void foo(int i) {\n"
              "    for (int c = 0; c <= i; c++)\n"
              "        x.y.z = 13;\n"
              "    int buf[10];\n"
              "    if (buf[i] > 0) { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:9]: (error) Array index out of bounds, cannot determine that i is less than 10\n"
                      "[test.cpp:9]: (error) Array index out of bounds, cannot determine that i is not negative\n"
                      "[test.cpp:9]: (error) Cannot determine that 'buf[i]' is initialized\n",
                      errout.str());
    }

    void arrayIndexOutOfBoundsDim1() { // itc test case
        check("void overrun_st_008 () {\n"
              "    int buf[5][6];\n"
              "    buf[5][5] = 1;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Array index out of bounds, cannot determine that 5 is less than 5\n", errout.str());
    }

    void bufferOverflowMemCmp1() {
        // CVE-2020-24265
        check("void foo(const char *pktdata, int datalen) {\n"
              "  if (memcmp(pktdata, \"MGC\", 3)) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (error) Buffer read/write, when calling 'memcmp' it cannot be determined that 1st argument is not overflowed\n", errout.str());
    }

    void bufferOverflowMemCmp2() {
        check("void foo(const char *pktdata, unsigned int datalen) {\n"
              "  if (memcmp(pktdata, \"MGC\", datalen)) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (error) Buffer read/write, when calling 'memcmp' it cannot be determined that 1st argument is not overflowed\n", errout.str());
    }

    void bufferOverflowStrcpy1() {
        check("void foo(char *p) {\n"
              "  strcpy(p, \"hello\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (error) Buffer read/write, when calling 'strcpy' it cannot be determined that 1st argument is not overflowed\n", errout.str());
    }

    void bufferOverflowStrcpy2() {
        check("void foo(char *p, const char *q) {\n"
              "  strcpy(p, q);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (error) Buffer read/write, when calling 'strcpy' it cannot be determined that 1st argument is not overflowed\n", errout.str());
    }


    void divisionByZeroNoReturn() {
        // Don't know if function is noreturn or not..
        check("int f(int leftarg, int rightarg) {\n"
              "  if (rightarg == 0)\n"
              "    raise (SIGFPE);\n"  // <- maybe noreturn
              "  return leftarg / rightarg;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) There is division, cannot determine that there can't be a division by zero.\n", errout.str());
    }


    void uninit() {
        check("void foo() { int x; x = x + 1; }");
        ASSERT_EQUALS("[test.cpp:1]: (error) Cannot determine that 'x' is initialized\n", errout.str());

        check("void foo() { int x; int y = x + 1; }");
        ASSERT_EQUALS("[test.cpp:1]: (error) Cannot determine that 'x' is initialized\n", errout.str());

        check("void foo() { int x; x++; }");
        ASSERT_EQUALS("[test.cpp:1]: (error) Cannot determine that 'x' is initialized\n", errout.str());
    }

    void uninit_array() {
        check("void foo(int x) {\n"
              "  int a[10];\n"
              "  if (x > 0) a[0] = 32;\n"
              "  return a[0];\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Cannot determine that 'a[0]' is initialized\n", errout.str());
    }

    void uninit_function_par() {
        // non constant parameters may point at uninitialized data
        // constant parameters should point at initialized data

        check("char foo(char id[]) { return id[0]; }");
        ASSERT_EQUALS("[test.cpp:1]: (error) Cannot determine that 'id[0]' is initialized (you can use 'const' to say data must be initialized)\n", errout.str());

        check("char foo(const char id[]) { return id[0]; }");
        ASSERT_EQUALS("", errout.str());

        check("char foo(const char id[]);\n"
              "void bar() { char data[10]; foo(data); }");
        ASSERT_EQUALS("[test.cpp:2]: (error) Cannot determine that 'data[0]' is initialized\n", errout.str());

        check("char foo(char id[]);\n"
              "void bar() { char data[10]; foo(data); }");
        ASSERT_EQUALS("[test.cpp:2]: (error, inconclusive) Cannot determine that 'data[0]' is initialized. It is inconclusive if there would be a problem in the function call.\n", errout.str());

        check("void foo(int *p) { if (p) *p=0; }");
        ASSERT_EQUALS("", errout.str());

        check("class C {\n"
              "public:\n"
              "  C();\n"
              "  int x;\n"
              "};\n"
              "\n"
              "void foo(const C &c) { int x = c.x; }");
        ASSERT_EQUALS("", errout.str());
    }

    void uninit_malloc() {
        check("void foo() { char *p = malloc(10); return *p; }");
        ASSERT_EQUALS("[test.cpp:1]: (error) Cannot determine that '*p' is initialized\n", errout.str());
    }

    void uninit_struct() {
        // Assume that constructors initialize all members
        // TODO whole program analysis
        check("struct Data { Data(); int x; }\n"
              "void foo() {\n"
              "  Data data;\n"
              "  x = data.x;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void uninit_bailout() {
        check("void foo() {\n"
              "    __CPPCHECK_BAILOUT__;\n"
              "    int values[5];\n"
              "    values[i] = 123;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo() {\n"
              "    __CPPCHECK_BAILOUT__;\n"
              "    std::ostringstream comm;\n"
              "    comm << 123;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void ctu() {
        check("void init(int &x) {\n"
              "    x = 1;\n"
              "}\n"
              "\n"
              "void foo() {\n"
              "    int x;\n"
              "    init(x);\n"
              "    x++;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void init(int a, int &x) {\n"
              "    if (a < 10)\n"
              "        x = 1;\n"
              "}\n"
              "\n"
              "void foo(int a) {\n"
              "    int x;\n"
              "    init(a, x);\n"
              "    x++;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:9]: (error) Cannot determine that 'x' is initialized\n", errout.str());

        check("void init(int a, int &x) {\n"
              "    if (a < 10)\n"
              "        x = 1;\n"
              "    else\n"
              "        x = 3;\n"
              "}\n"
              "\n"
              "void foo(int a) {\n"
              "    int x;\n"
              "    init(a, x);\n"
              "    x++;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void uninit_fp_smartptr() {
        check("void foo() {\n"
              "    std::unique_ptr<std::string> buffer;\n"
              "    try { } catch (std::exception& e) { }\n"
              "    doneCallback(std::move(buffer));\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void uninit_fp_struct() {
        check("struct Pos {\n"
              "    int x {0};\n"
              "    int y {0};\n"
              "};\n"
              "\n"
              "void dostuff() {\n"
              "    auto obj = C {};\n"
              "    Pos xy;\n"
              "    foo(xy);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void uninit_fp_struct_member_init_2() {
        check("struct A {\n"
              "    int x {0}; int y {0};\n"
              "};\n"
              "void foo(const A& a) {\n"
              "    bar(a);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void uninit_fp_template_var() {
        check("void foo() {\n"
              "    X*x = DYNAMIC_CAST(X, p);\n"
              "    C<int> c;\n"
              "    f(c);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }
};

REGISTER_TEST(TestBughuntingChecks)
