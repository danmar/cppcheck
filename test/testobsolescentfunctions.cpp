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
#include "checkobsolescentfunctions.h"
#include "testsuite.h"


class TestObsoleteFunctions : public TestFixture {
public:
    TestObsoleteFunctions() : TestFixture("TestObsoleteFunctions") {
    }

private:

    void run() {
        TEST_CASE(testbsd_signal);
        TEST_CASE(testgethostbyname);
        TEST_CASE(testgethostbyaddr);
        TEST_CASE(testusleep);
        TEST_CASE(testindex);
        TEST_CASE(test_qt_index); // FP when using the Qt function 'index'?
        TEST_CASE(testrindex);

        // no false positives for variables
        TEST_CASE(testvar);

        // dangerous function
        TEST_CASE(testgets);

        TEST_CASE(testalloca);

        // declared function ticket #3121
        TEST_CASE(test_declared_function);

        // test std::gets
        TEST_CASE(test_std_gets);

        // multiple use of obsolete functions
        TEST_CASE(test_multiple);

        // c declared function
        TEST_CASE(test_c_declaration);

        // function with body
        TEST_CASE(test_function_with_body);

        // null pointer dereference in obsoleteFunctions
        TEST_CASE(ticket3238);
    }

    void check(const char code[], const char filename[]="test.cpp") {
        // Clear the error buffer..
        errout.str("");

        Settings settings;
        settings.addEnabled("style");
        settings.standards.posix = true;
        settings.standards.c = Standards::C11;

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, filename);
        tokenizer.simplifyTokenList2();

        // Check for obsolete functions..
        CheckObsoleteFunctions checkObsoleteFunctions(&tokenizer, &settings, this);
        checkObsoleteFunctions.obsoleteFunctions();
    }

    void testbsd_signal() {
        check("void f()\n"
              "{\n"
              "    bsd_signal(SIGABRT, SIG_IGN);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Obsolete function 'bsd_signal' called. It is recommended to use the function 'sigaction' instead.\n", errout.str());

        check("int f()\n"
              "{\n"
              "    int bsd_signal(0);\n"
              "    return bsd_signal;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }


    void testgethostbyname() {
        check("void f()\n"
              "{\n"
              "    struct hostent *hp;\n"
              "    if(!hp = gethostbyname('127.0.0.1')) {\n"
              "        exit(1);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) Obsolete function 'gethostbyname' called. It is recommended to use the function 'getaddrinfo' instead.\n", errout.str());
    }

    void testgethostbyaddr() {
        check("void f()\n"
              "{\n"
              "    long addr;\n"
              "    addr = inet_addr('127.0.0.1');\n"
              "    if(!hp = gethostbyaddr((char *) &addr, sizeof(addr), AF_INET)) {\n"
              "        exit(1);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (style) Obsolete function 'gethostbyaddr' called. It is recommended to use the function 'getnameinfo' instead.\n", errout.str());
    }

    void testusleep() {
        check("void f()\n"
              "{\n"
              "    usleep( 1000 );\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Obsolete function 'usleep' called. It is recommended to use the 'nanosleep' or 'setitimer' function instead.\n", errout.str());
    }

    void testindex() {

        check("namespace n1 {\n"
              "    int index(){};\n"
              "}\n"
              "int main()\n"
              "{\n"
              "    n1::index();\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("std::size_t f()\n"
              "{\n"
              "    std::size_t index(0);\n"
              "    index++;\n"
              "    return index;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("int f()\n"
              "{\n"
              "    return this->index();\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              "    int index( 0 );\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("const char f()\n"
              "{\n"
              "    const char var[6] = 'index';\n"
              "    const char i = index(var, 0);\n"
              "    return i;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) Obsolete function 'index' called. It is recommended to use the function 'strchr' instead.\n",
                      errout.str());
    }

    void test_qt_index() {
        check("void TDataModel::forceRowRefresh(int row) {\n"
              "    emit dataChanged(index(row, 0), index(row, columnCount() - 1));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Obsolete function 'index' called. It is recommended to use the function 'strchr' instead.\n", errout.str());
    }

    void testrindex() {
        check("void f()\n"
              "{\n"
              "    int rindex( 0 );\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              "    const char var[7] = 'rindex';\n"
              "    print(rindex(var, 0));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) Obsolete function 'rindex' called. It is recommended to use the function 'strrchr' instead.\n", errout.str());
    }


    void testvar() {
        check("class Fred {\n"
              "public:\n"
              "    Fred() : index(0) { }\n"
              "    int index;\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void testgets() {
        check("void f()\n"
              "{\n"
              "    char *x = gets();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Obsolete function 'gets' called. It is recommended to use the function 'fgets' instead.\n", errout.str());

        check("void f()\n"
              "{\n"
              "    foo(x, gets());\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Obsolete function 'gets' called. It is recommended to use the function 'fgets' instead.\n", errout.str());
    }

    void testalloca() {
        check("void f()\n"
              "{\n"
              "    char *x = alloca(10);\n"
              "}\n", "test.cpp");  // #4382 - there are no VLAs in C++
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              "    char *x = alloca(10);\n"
              "}\n", "test.c");
        ASSERT_EQUALS("[test.c:3]: (style) Obsolete function 'alloca' called. In C99 and later it is recommended to use a variable length array instead.\n", errout.str());
    }

    // ticket #3121
    void test_declared_function() {
        check("int ftime ( int a )\n"
              "{\n"
              "    return a;\n"
              "}\n"
              "int main ()\n"
              "{\n"
              "    int b ; b = ftime ( 1 ) ;\n"
              "    return 0 ;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    // test std::gets
    void test_std_gets() {
        check("void f(char * str)\n"
              "{\n"
              "    char *x = std::gets(str);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Obsolete function 'gets' called. It is recommended to use the function 'fgets' instead.\n", errout.str());
    }

    // multiple use
    void test_multiple() {
        check("void f(char * str)\n"
              "{\n"
              "    char *x = std::gets(str);\n"
              "    usleep( 1000 );\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Obsolete function 'gets' called. It is recommended to use the function 'fgets' instead.\n"
                      "[test.cpp:4]: (style) Obsolete function 'usleep' called. It is recommended to use the 'nanosleep' or 'setitimer' function instead.\n", errout.str());
    }

    void test_c_declaration() {
        check("char * gets ( char * c ) ;\n"
              "int main ()\n"
              "{\n"
              "    char s [ 10 ] ;\n"
              "    gets ( s ) ;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (style) Obsolete function 'gets' called. It is recommended to use the function 'fgets' instead.\n", errout.str());

        check("int getcontext(ucontext_t *ucp);\n"
              "int f (ucontext_t *ucp)\n"
              "{\n"
              "    getcontext ( ucp ) ;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) Obsolete function 'getcontext' called. Due to portability issues, applications are recommended to be rewritten to use POSIX threads.\n", errout.str());
    }

    void test_function_with_body() {
        check("char * gets ( char * c ) { return c; }\n"
              "int main ()\n"
              "{\n"
              "    char s [ 10 ] ;\n"
              "    gets ( s ) ;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void ticket3238() {
        check("__FBSDID(\"...\");\n");
        ASSERT_EQUALS("", errout.str());
    }

};

REGISTER_TEST(TestObsoleteFunctions)
