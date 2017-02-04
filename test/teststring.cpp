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
#include "checkstring.h"
#include "testsuite.h"
#include "testutils.h"


class TestString : public TestFixture {
public:
    TestString() : TestFixture("TestString") {
    }

private:
    Settings settings;

    void run() {
        settings.addEnabled("warning");
        settings.addEnabled("style");

        TEST_CASE(stringLiteralWrite);

        TEST_CASE(alwaysTrueFalseStringCompare);
        TEST_CASE(suspiciousStringCompare);
        TEST_CASE(suspiciousStringCompare_char);

        TEST_CASE(strPlusChar1);     // "/usr" + '/'
        TEST_CASE(strPlusChar2);     // "/usr" + ch
        TEST_CASE(strPlusChar3);     // ok: path + "/sub" + '/'

        TEST_CASE(sprintf1);        // Dangerous usage of sprintf
        TEST_CASE(sprintf2);
        TEST_CASE(sprintf3);
        TEST_CASE(sprintf4);        // struct member

        TEST_CASE(incorrectStringCompare);
    }

    void check(const char code[], const char filename[] = "test.cpp") {
        // Clear the error buffer..
        errout.str("");

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, filename);

        // Check char variable usage..
        CheckString checkString(&tokenizer, &settings, this);
        checkString.runChecks(&tokenizer, &settings, this);

        tokenizer.simplifyTokenList2();
        checkString.runSimplifiedChecks(&tokenizer, &settings, this);
    }

    void stringLiteralWrite() {
        check("void f() {\n"
              "  char *abc = \"abc\";\n"
              "  abc[0] = 'a';\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:2]: (error) Modifying string literal \"abc\" directly or indirectly is undefined behaviour.\n", errout.str());

        check("void f() {\n"
              "  char *abc = \"abc\";\n"
              "  *abc = 'a';\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3] -> [test.cpp:2]: (error) Modifying string literal \"abc\" directly or indirectly is undefined behaviour.\n", errout.str());

        check("void f() {\n"
              "  QString abc = \"abc\";\n"
              "  abc[0] = 'a';\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo_FP1(char *p) {\n"
              "  p[1] = 'B';\n"
              "}\n"
              "void foo_FP2(void) {\n"
              "  char* s = \"Y\";\n"
              "  foo_FP1(s);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2] -> [test.cpp:5]: (error) Modifying string literal \"Y\" directly or indirectly is undefined behaviour.\n", errout.str());

        check("void foo_FP1(char *p) {\n"
              "  p[1] = 'B';\n"
              "}\n"
              "void foo_FP2(void) {\n"
              "  char s[10] = \"Y\";\n"
              "  foo_FP1(s);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void alwaysTrueFalseStringCompare() {
        check("void f() {\n"
              "  if (strcmp(\"A\",\"A\")){}\n"
              "  if (strncmp(\"A\",\"A\",1)){}\n"
              "  if (strcasecmp(\"A\",\"A\")){}\n"
              "  if (strncasecmp(\"A\",\"A\",1)){}\n"
              "  if (memcmp(\"A\",\"A\",1)){}\n"
              "  if (strverscmp(\"A\",\"A\")){}\n"
              "  if (bcmp(\"A\",\"A\",1)){}\n"
              "  if (wcsncasecmp(L\"A\",L\"A\",1)){}\n"
              "  if (wcsncmp(L\"A\",L\"A\",1)){}\n"
              "  if (wmemcmp(L\"A\",L\"A\",1)){}\n"
              "  if (wcscmp(L\"A\",L\"A\")){}\n"
              "  if (wcscasecmp(L\"A\",L\"A\")){}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Unnecessary comparison of static strings.\n"
                      "[test.cpp:3]: (warning) Unnecessary comparison of static strings.\n"
                      "[test.cpp:4]: (warning) Unnecessary comparison of static strings.\n"
                      "[test.cpp:5]: (warning) Unnecessary comparison of static strings.\n"
                      "[test.cpp:6]: (warning) Unnecessary comparison of static strings.\n"
                      "[test.cpp:7]: (warning) Unnecessary comparison of static strings.\n"
                      "[test.cpp:8]: (warning) Unnecessary comparison of static strings.\n"
                      "[test.cpp:9]: (warning) Unnecessary comparison of static strings.\n"
                      "[test.cpp:10]: (warning) Unnecessary comparison of static strings.\n"
                      "[test.cpp:11]: (warning) Unnecessary comparison of static strings.\n"
                      "[test.cpp:12]: (warning) Unnecessary comparison of static strings.\n"
                      "[test.cpp:13]: (warning) Unnecessary comparison of static strings.\n", errout.str());

        // avoid false positives when the address is modified #6415
        check("void f(void *p, int offset)  {\n"
              "     if (!memcmp(p, p + offset, 42)){}\n"
              "     if (!memcmp(p + offset, p, 42)){}\n"
              "     if (!memcmp(offset + p, p, 42)){}\n"
              "     if (!memcmp(p, offset + p, 42)){}\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // avoid false positives when the address is modified #6415
        check("void f(char *c, int offset)  {\n"
              "     if (!memcmp(c, c + offset, 42)){}\n"
              "     if (!memcmp(c + offset, c, 42)){}\n"
              "     if (!memcmp(offset + c, c, 42)){}\n"
              "     if (!memcmp(c, offset + c, 42)){}\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // avoid false positives when the address is modified #6415
        check("void f(std::string s, int offset)  {\n"
              "     if (!memcmp(s.c_str(), s.c_str() + offset, 42)){}\n"
              "     if (!memcmp(s.c_str() + offset, s.c_str(), 42)){}\n"
              "     if (!memcmp(offset + s.c_str(), s.c_str(), 42)){}\n"
              "     if (!memcmp(s.c_str(), offset + s.c_str(), 42)){}\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("int main()\n"
              "{\n"
              "  if (strcmp(\"00FF00\", \"00FF00\") == 0)"
              "  {"
              "    std::cout << \"Equal\n\""
              "  }"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Unnecessary comparison of static strings.\n", errout.str());

        check("void f() {\n"
              "  if (strcmp($\"00FF00\", \"00FF00\") == 0) {}"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "  if ($strcmp(\"00FF00\", \"00FF00\") == 0) {}"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("int main()\n"
              "{\n"
              "  if (stricmp(\"hotdog\",\"HOTdog\") == 0)"
              "  {"
              "    std::cout << \"Equal\n\""
              "  }"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Unnecessary comparison of static strings.\n", errout.str());

        check("int main()\n"
              "{\n"
              "  if (QString::compare(\"Hamburger\", \"Hotdog\") == 0)"
              "  {"
              "    std::cout << \"Equal\n\""
              "  }"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Unnecessary comparison of static strings.\n", errout.str());

        check("int main()\n"
              "{\n"
              "  if (QString::compare(argv[2], \"hotdog\") == 0)"
              "  {"
              "    std::cout << \"Equal\n\""
              "  }"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("int main()\n"
              "{\n"
              "  if (strncmp(\"hotdog\",\"hotdog\", 6) == 0)"
              "  {"
              "    std::cout << \"Equal\n\""
              "  }"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Unnecessary comparison of static strings.\n", errout.str());

        check("int foo(const char *buf)\n"
              "{\n"
              "  if (strcmp(buf, buf) == 0)"
              "  {"
              "    std::cout << \"Equal\n\""
              "  }"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Comparison of identical string variables.\n", errout.str());

        check("int foo(const std::string& buf)\n"
              "{\n"
              "  if (stricmp(buf.c_str(), buf.c_str()) == 0)"
              "  {"
              "    std::cout << \"Equal\n\""
              "  }"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Comparison of identical string variables.\n", errout.str());

        check("int main() {\n"
              "  if (\"str\" == \"str\") {\n"
              "    std::cout << \"Equal\n\"\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Unnecessary comparison of static strings.\n", errout.str());

        check("int main() {\n"
              "  if (\"str\" != \"str\") {\n"
              "    std::cout << \"Equal\n\"\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Unnecessary comparison of static strings.\n", errout.str());

        check("int main() {\n"
              "  if (a+\"str\" != \"str\"+b) {\n"
              "    std::cout << \"Equal\n\"\n"
              "  }\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void suspiciousStringCompare() {
        check("bool foo(char* c) {\n"
              "    return c == \"x\";\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) String literal compared with variable 'c'. Did you intend to use strcmp() instead?\n", errout.str());

        check("bool foo(const char* c) {\n"
              "    return \"x\" == c;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) String literal compared with variable 'c'. Did you intend to use strcmp() instead?\n", errout.str());

        check("bool foo(char* c) {\n"
              "    return foo+\"x\" == c;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("bool foo(char* c) {\n"
              "    return \"x\" == c+foo;\n"
              "}", "test.cpp");
        ASSERT_EQUALS("", errout.str());

        check("bool foo(char* c) {\n"
              "    return \"x\" == c+foo;\n"
              "}", "test.c");
        ASSERT_EQUALS("[test.c:2]: (warning) String literal compared with variable 'c'. Did you intend to use strcmp() instead?\n", errout.str());

        check("bool foo(Foo c) {\n"
              "    return \"x\" == c.foo;\n"
              "}", "test.cpp");
        ASSERT_EQUALS("", errout.str());

        check("bool foo(Foo c) {\n"
              "    return \"x\" == c.foo;\n"
              "}", "test.c");
        ASSERT_EQUALS("[test.c:2]: (warning) String literal compared with variable 'c.foo'. Did you intend to use strcmp() instead?\n", errout.str());

        check("bool foo(const std::string& c) {\n"
              "    return \"x\" == c;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("bool foo(const Foo* c) {\n"
              "    return \"x\" == c->bar();\n" // #4314
              "}");
        ASSERT_EQUALS("", errout.str());

        // Ticket #4257
        check("bool foo() {\n"
              "MyString *str=Getter();\n"
              "return *str==\"bug\"; }\n", "test.c");
        ASSERT_EQUALS("[test.c:3]: (warning) String literal compared with variable '*str'. Did you intend to use strcmp() instead?\n", errout.str());

        // Ticket #4257
        check("bool foo() {\n"
              "MyString *str=Getter();\n"
              "return *str==\"bug\"; }");
        ASSERT_EQUALS("", errout.str());

        // Ticket #4257
        check("bool foo() {\n"
              "MyString **str=OtherGetter();\n"
              "return *str==\"bug\"; }");
        TODO_ASSERT_EQUALS("[test.cpp:2]: (warning) String literal compared with variable 'c'. Did you intend to use strcmp() instead?\n",
                           "",
                           errout.str());

        // Ticket #4257
        check("bool foo() {\n"
              "MyString str=OtherGetter2();\n"
              "return &str==\"bug\"; }");
        TODO_ASSERT_EQUALS("[test.cpp:2]: (warning) String literal compared with variable 'c'. Did you intend to use strcmp() instead?\n",
                           "",
                           errout.str());

        // Ticket #5734
        check("int foo(char c) {\n"
              "return c == '4';}", "test.cpp");
        ASSERT_EQUALS("", errout.str());
        check("int foo(char c) {\n"
              "return c == '4';}", "test.c");
        ASSERT_EQUALS("", errout.str());
        check("int foo(char c) {\n"
              "return c == \"42\"[0];}", "test.cpp");
        ASSERT_EQUALS("", errout.str());
        check("int foo(char c) {\n"
              "return c == \"42\"[0];}", "test.c");
        ASSERT_EQUALS("", errout.str());

        // 5639 String literal compared with char buffer in a struct
        check("struct Example {\n"
              "   char buffer[200];\n"
              "};\n"
              "void foo() {\n"
              "   struct Example example;\n"
              "   if (example.buffer == \"test\") ;\n"
              "}\n", "test.cpp");
        ASSERT_EQUALS("[test.cpp:6]: (warning) String literal compared with variable 'example.buffer'. Did you intend to use strcmp() instead?\n", errout.str());
        check("struct Example {\n"
              "   char buffer[200];\n"
              "};\n"
              "void foo() {\n"
              "   struct Example example;\n"
              "   if (example.buffer == \"test\") ;\n"
              "}\n", "test.c");
        ASSERT_EQUALS("[test.c:6]: (warning) String literal compared with variable 'example.buffer'. Did you intend to use strcmp() instead?\n", errout.str());
    }

    void suspiciousStringCompare_char() {
        check("bool foo(char* c) {\n"
              "    return c == 'x';\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Char literal compared with pointer 'c'. Did you intend to dereference it?\n", errout.str());

        check("bool foo(char* c) {\n"
              "    return '\\0' != c;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Char literal compared with pointer 'c'. Did you intend to dereference it?\n", errout.str());

        check("bool foo(char c) {\n"
              "    return c == '\\0';\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("bool foo(char c) {\n"
              "    return c == 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("bool foo(char* c) {\n"
              "    return *c == 0;\n"
              "}", "test.c");
        ASSERT_EQUALS("", errout.str());

        check("bool foo(char* c) {\n"
              "    return *c == 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("bool foo(Foo* c) {\n"
              "    return 0 == c->x;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo(char* c) {\n"
              "    if(c == '\\0') bar();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Char literal compared with pointer 'c'. Did you intend to dereference it?\n", errout.str());

        check("void f() {\n"
              "  struct { struct { char *str; } x; } a;\n"
              "  return a.x.str == '\\0';"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Char literal compared with pointer 'a.x.str'. Did you intend to dereference it?\n", errout.str());

        check("void f() {\n"
              "  struct { struct { char *str; } x; } a;\n"
              "  return *a.x.str == '\\0';"
              "}");
        ASSERT_EQUALS("", errout.str());
    }


    void sprintf1() {
        check("void foo()\n"
              "{\n"
              "    char buf[100];\n"
              "    sprintf(buf,\"%s\",buf);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Undefined behavior: Variable 'buf' is used as parameter and destination in s[n]printf().\n", errout.str());
    }

    void sprintf2() {
        check("void foo()\n"
              "{\n"
              "    char buf[100];\n"
              "    sprintf(buf,\"%i\",sizeof(buf));\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void sprintf3() {
        check("void foo()\n"
              "{\n"
              "    char buf[100];\n"
              "    sprintf(buf,\"%i\",sizeof(buf));\n"
              "    if (buf[0]);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void sprintf4() {
        check("struct A\n"
              "{\n"
              "    char filename[128];\n"
              "};\n"
              "\n"
              "void foo()\n"
              "{\n"
              "    const char* filename = \"hello\";\n"
              "    struct A a;\n"
              "    snprintf(a.filename, 128, \"%s\", filename);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void strPlusChar1() {
        // Strange looking pointer arithmetic..
        check("void foo()\n"
              "{\n"
              "    const char *p = \"/usr\" + '/';\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Unusual pointer arithmetic. A value of type 'char' is added to a string literal.\n", errout.str());
    }

    void strPlusChar2() {
        // Strange looking pointer arithmetic..
        check("void foo()\n"
              "{\n"
              "    char ch = 1;\n"
              "    const char *p = ch + \"/usr\";\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // Strange looking pointer arithmetic..
        check("void foo()\n"
              "{\n"
              "    int i = 1;\n"
              "    const char* psz = \"Bla\";\n"
              "    const std::string str = i + psz;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void strPlusChar3() {
        // Strange looking pointer arithmetic..
        check("void foo()\n"
              "{\n"
              "    std::string temp = \"/tmp\";\n"
              "    std::string path = temp + '/' + \"sub\" + '/';\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }


    void incorrectStringCompare() {
        check("int f() {\n"
              "    return test.substr( 0 , 4 ) == \"Hello\" ? : 0 : 1 ;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) String literal \"Hello\" doesn't match length argument for substr().\n", errout.str());

        check("int f() {\n"
              "    return test.substr( 0 , 5 ) == \"Hello\" ? : 0 : 1 ;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("int f() {\n"
              "    return \"Hello\" == test.substr( 0 , 4 ) ? : 0 : 1 ;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) String literal \"Hello\" doesn't match length argument for substr().\n", errout.str());

        check("int f() {\n"
              "    return \"Hello\" == foo.bar<int>().z[1].substr(i+j*4, 4) ? : 0 : 1 ;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) String literal \"Hello\" doesn't match length argument for substr().\n", errout.str());

        check("int f() {\n"
              "    return \"Hello\" == test.substr( 0 , 5 ) ? : 0 : 1 ;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("int f() {\n"
              "    if (\"Hello\") { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Conversion of string literal \"Hello\" to bool always evaluates to true.\n", errout.str());

        check("int f() {\n"
              "    if (\"Hello\" && test) { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Conversion of string literal \"Hello\" to bool always evaluates to true.\n", errout.str());

        check("int f() {\n"
              "    if (test && \"Hello\") { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Conversion of string literal \"Hello\" to bool always evaluates to true.\n", errout.str());

        check("int f() {\n"
              "    while (\"Hello\") { }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Conversion of string literal \"Hello\" to bool always evaluates to true.\n", errout.str());

        check("int f() {\n"
              "    assert (test || \"Hello\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Conversion of string literal \"Hello\" to bool always evaluates to true.\n", errout.str());

        check("int f() {\n"
              "    assert (test && \"Hello\");\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("int f() {\n"
              "    assert (\"Hello\" || test);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) Conversion of string literal \"Hello\" to bool always evaluates to true.\n", errout.str());

        check("int f() {\n"
              "    assert (\"Hello\" && test);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("int f() {\n"
              "    BOOST_ASSERT (\"Hello\" && test);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("int f() {\n"
              "    return f2(\"Hello\");\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }
};

REGISTER_TEST(TestString)
