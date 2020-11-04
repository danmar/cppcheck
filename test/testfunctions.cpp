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

#include <tinyxml2.h>

#include "checkfunctions.h"
#include "library.h"
#include "settings.h"
#include "standards.h"
#include "testsuite.h"
#include "tokenize.h"


class TestFunctions : public TestFixture {
public:
    TestFunctions() : TestFixture("TestFunctions") {
    }

private:
    Settings settings;

    void run() OVERRIDE {
        settings.addEnabled("style");
        settings.addEnabled("warning");
        settings.addEnabled("portability");
        settings.libraries.emplace_back("posix");
        settings.standards.c = Standards::C11;
        settings.standards.cpp = Standards::CPP11;
        LOAD_LIB_2(settings.library, "std.cfg");
        LOAD_LIB_2(settings.library, "posix.cfg");

        // Prohibited functions
        TEST_CASE(prohibitedFunctions_posix);
        TEST_CASE(prohibitedFunctions_index);
        TEST_CASE(prohibitedFunctions_qt_index); // FP when using the Qt function 'index'?
        TEST_CASE(prohibitedFunctions_rindex);
        TEST_CASE(prohibitedFunctions_var); // no false positives for variables
        TEST_CASE(prohibitedFunctions_gets); // dangerous function
        TEST_CASE(prohibitedFunctions_alloca);
        TEST_CASE(prohibitedFunctions_declaredFunction); // declared function ticket #3121
        TEST_CASE(prohibitedFunctions_std_gets); // test std::gets
        TEST_CASE(prohibitedFunctions_multiple); // multiple use of obsolete functions
        TEST_CASE(prohibitedFunctions_c_declaration); // c declared function
        TEST_CASE(prohibitedFunctions_functionWithBody); // function with body
        TEST_CASE(prohibitedFunctions_crypt); // Non-reentrant function
        TEST_CASE(prohibitedFunctions_namespaceHandling);

        // Invalid function usage
        TEST_CASE(invalidFunctionUsage1);
        // TODO TEST_CASE(invalidFunctionUsageStrings);

        // Math function usage
        TEST_CASE(mathfunctionCall_fmod);
        TEST_CASE(mathfunctionCall_sqrt);
        TEST_CASE(mathfunctionCall_log);
        TEST_CASE(mathfunctionCall_acos);
        TEST_CASE(mathfunctionCall_asin);
        TEST_CASE(mathfunctionCall_pow);
        TEST_CASE(mathfunctionCall_atan2);
        TEST_CASE(mathfunctionCall_precision);

        // Ignored return value
        TEST_CASE(checkIgnoredReturnValue);
        TEST_CASE(checkIgnoredErrorCode);

        // memset..
        TEST_CASE(memsetZeroBytes);
        TEST_CASE(memsetInvalid2ndParam);
    }

    void check(const char code[], const char filename[]="test.cpp", const Settings* settings_=nullptr) {
        // Clear the error buffer..
        errout.str("");

        if (!settings_)
            settings_ = &settings;

        // Tokenize..
        Tokenizer tokenizer(settings_, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, filename);

        CheckFunctions checkFunctions(&tokenizer, settings_, this);
        checkFunctions.runChecks(&tokenizer, settings_, this);
    }

    void prohibitedFunctions_posix() {
        check("void f()\n"
              "{\n"
              "    bsd_signal(SIGABRT, SIG_IGN);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Obsolescent function 'bsd_signal' called. It is recommended to use 'sigaction' instead.\n", errout.str());

        check("int f()\n"
              "{\n"
              "    int bsd_signal(0);\n"
              "    return bsd_signal;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              "    struct hostent *hp;\n"
              "    if(!hp = gethostbyname(\"127.0.0.1\")) {\n"
              "        exit(1);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) Obsolescent function 'gethostbyname' called. It is recommended to use 'getaddrinfo' instead.\n", errout.str());

        check("void f()\n"
              "{\n"
              "    long addr;\n"
              "    addr = inet_addr(\"127.0.0.1\");\n"
              "    if(!hp = gethostbyaddr((char *) &addr, sizeof(addr), AF_INET)) {\n"
              "        exit(1);\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (style) Obsolescent function 'gethostbyaddr' called. It is recommended to use 'getnameinfo' instead.\n", errout.str());

        check("void f()\n"
              "{\n"
              "    usleep( 1000 );\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (style) Obsolescent function 'usleep' called. It is recommended to use 'nanosleep' or 'setitimer' instead.\n", errout.str());
    }

    void prohibitedFunctions_index() {
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
              "    const char var[6] = \"index\";\n"
              "    const char i = index(var, 0);\n"
              "    return i;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) Obsolescent function 'index' called. It is recommended to use 'strchr' instead.\n",
                      errout.str());
    }

    void prohibitedFunctions_qt_index() {
        check("void TDataModel::forceRowRefresh(int row) {\n"
              "    emit dataChanged(index(row, 0), index(row, columnCount() - 1));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Obsolescent function 'index' called. It is recommended to use 'strchr' instead.\n", errout.str());
    }

    void prohibitedFunctions_rindex() {
        check("void f()\n"
              "{\n"
              "    int rindex( 0 );\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              "    const char var[7] = \"rindex\";\n"
              "    print(rindex(var, 0));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (style) Obsolescent function 'rindex' called. It is recommended to use 'strrchr' instead.\n", errout.str());
    }


    void prohibitedFunctions_var() {
        check("class Fred {\n"
              "public:\n"
              "    Fred() : index(0) { }\n"
              "    int index;\n"
              "};");
        ASSERT_EQUALS("", errout.str());
    }

    void prohibitedFunctions_gets() {
        check("void f()\n"
              "{\n"
              "    char *x = gets(a);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Obsolete function 'gets' called. It is recommended to use 'fgets' or 'gets_s' instead.\n", errout.str());

        check("void f()\n"
              "{\n"
              "    foo(x, gets(a));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Obsolete function 'gets' called. It is recommended to use 'fgets' or 'gets_s' instead.\n", errout.str());
    }

    void prohibitedFunctions_alloca() {
        check("void f()\n"
              "{\n"
              "    char *x = alloca(10);\n"
              "}", "test.cpp");  // #4382 - there are no VLAs in C++
        ASSERT_EQUALS("[test.cpp:3]: (warning) Obsolete function 'alloca' called.\n", errout.str());

        check("void f()\n"
              "{\n"
              "    char *x = alloca(10);\n"
              "}", "test.c");
        ASSERT_EQUALS("[test.c:3]: (warning) Obsolete function 'alloca' called. In C99 and later it is recommended to use a variable length array instead.\n", errout.str());

        settings.standards.c = Standards::C89;
        settings.standards.cpp = Standards::CPP03;
        check("void f()\n"
              "{\n"
              "    char *x = alloca(10);\n"
              "}", "test.cpp");  // #4382 - there are no VLAs in C++
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              "    char *x = alloca(10);\n"
              "}", "test.c"); // #7558 - no alternative to alloca in C89
        ASSERT_EQUALS("", errout.str());

        check("void f()\n"
              "{\n"
              "    char *x = alloca(10);\n"
              "}", "test.c");
        ASSERT_EQUALS("", errout.str());
        settings.standards.c = Standards::C11;
        settings.standards.cpp = Standards::CPP11;
    }

    // ticket #3121
    void prohibitedFunctions_declaredFunction() {
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
    void prohibitedFunctions_std_gets() {
        check("void f(char * str)\n"
              "{\n"
              "    char *x = std::gets(str);\n"
              "    char *y = gets(str);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Obsolete function 'gets' called. It is recommended to use 'fgets' or 'gets_s' instead.\n"
                      "[test.cpp:4]: (warning) Obsolete function 'gets' called. It is recommended to use 'fgets' or 'gets_s' instead.\n", errout.str());
    }

    // multiple use
    void prohibitedFunctions_multiple() {
        check("void f(char * str)\n"
              "{\n"
              "    char *x = std::gets(str);\n"
              "    usleep( 1000 );\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Obsolete function 'gets' called. It is recommended to use 'fgets' or 'gets_s' instead.\n"
                      "[test.cpp:4]: (style) Obsolescent function 'usleep' called. It is recommended to use 'nanosleep' or 'setitimer' instead.\n", errout.str());
    }

    void prohibitedFunctions_c_declaration() {
        check("char * gets ( char * c ) ;\n"
              "int main ()\n"
              "{\n"
              "    char s [ 10 ] ;\n"
              "    gets ( s ) ;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (warning) Obsolete function 'gets' called. It is recommended to use 'fgets' or 'gets_s' instead.\n", errout.str());

        check("int getcontext(ucontext_t *ucp);\n"
              "int f (ucontext_t *ucp)\n"
              "{\n"
              "    getcontext ( ucp ) ;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (portability) Obsolescent function 'getcontext' called. Applications are recommended to be rewritten to use POSIX threads.\n", errout.str());
    }

    void prohibitedFunctions_functionWithBody() {
        check("char * gets ( char * c ) { return c; }\n"
              "int main ()\n"
              "{\n"
              "    char s [ 10 ] ;\n"
              "    gets ( s ) ;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void prohibitedFunctions_crypt() {
        check("void f(char *pwd)\n"
              "{\n"
              "    char *cpwd;"
              "    crypt(pwd, cpwd);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Return value of function crypt() is not used.\n"
                      "[test.cpp:3]: (portability) Non reentrant function 'crypt' called. For threadsafe applications it is recommended to use the reentrant replacement function 'crypt_r'.\n", errout.str());

        check("void f()\n"
              "{\n"
              "    char *pwd = getpass(\"Password:\");"
              "    char *cpwd;"
              "    crypt(pwd, cpwd);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Return value of function crypt() is not used.\n"
                      "[test.cpp:3]: (portability) Non reentrant function 'crypt' called. For threadsafe applications it is recommended to use the reentrant replacement function 'crypt_r'.\n", errout.str());

        check("int f()\n"
              "{\n"
              "    int crypt = 0;"
              "    return crypt;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void prohibitedFunctions_namespaceHandling() {
        check("int f()\n"
              "{\n"
              "    time_t t = 0;"
              "    std::localtime(&t);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (portability) Non reentrant function 'localtime' called. For threadsafe applications it is recommended to use the reentrant replacement function 'localtime_r'.\n", errout.str());

        // Passed as function argument
        check("int f()\n"
              "{\n"
              "    printf(\"Magic guess: %d\", getpwent());\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (portability) Non reentrant function 'getpwent' called. For threadsafe applications it is recommended to use the reentrant replacement function 'getpwent_r'.\n", errout.str());

        // Pass return value
        check("int f()\n"
              "{\n"
              "    time_t t = 0;"
              "    struct tm *foo = localtime(&t);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (portability) Non reentrant function 'localtime' called. For threadsafe applications it is recommended to use the reentrant replacement function 'localtime_r'.\n", errout.str());

        // Access via global namespace
        check("int f()\n"
              "{\n"
              "    ::getpwent();\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Return value of function getpwent() is not used.\n"
                      "[test.cpp:3]: (portability) Non reentrant function 'getpwent' called. For threadsafe applications it is recommended to use the reentrant replacement function 'getpwent_r'.\n", errout.str());

        // Be quiet on function definitions
        check("int getpwent()\n"
              "{\n"
              "    return 123;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // Be quiet on other namespaces
        check("int f()\n"
              "{\n"
              "    foobar::getpwent();\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // Be quiet on class member functions
        check("int f()\n"
              "{\n"
              "    foobar.getpwent();\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void invalidFunctionUsage1() {
        check("int f() { memset(a,b,sizeof(a)!=12); }");
        ASSERT_EQUALS("[test.cpp:1]: (error) Invalid memset() argument nr 3. A non-boolean value is required.\n", errout.str());

        check("int f() { memset(a,b,sizeof(a)!=0); }");
        ASSERT_EQUALS("[test.cpp:1]: (error) Invalid memset() argument nr 3. A non-boolean value is required.\n", errout.str());

        check("int f() { memset(a,b,!c); }");
        ASSERT_EQUALS("[test.cpp:1]: (error) Invalid memset() argument nr 3. A non-boolean value is required.\n", errout.str());

        // Ticket #6990
        check("int f(bool c) { memset(a,b,c); }");
        ASSERT_EQUALS("[test.cpp:1]: (error) Invalid memset() argument nr 3. A non-boolean value is required.\n", errout.str());
        check("int f() { memset(a,b,true); }");
        ASSERT_EQUALS("[test.cpp:1]: (error) Invalid memset() argument nr 3. A non-boolean value is required.\n", errout.str());

        // Ticket #6588 (c mode)
        check("void record(char* buf, int n) {\n"
              "  memset(buf, 0, n < 255);\n"           /* KO */
              "  memset(buf, 0, n < 255 ? n : 255);\n" /* OK */
              "}", "test.c");
        ASSERT_EQUALS("[test.c:2]: (error) Invalid memset() argument nr 3. A non-boolean value is required.\n", errout.str());

        // Ticket #6588 (c++ mode)
        check("void record(char* buf, int n) {\n"
              "  memset(buf, 0, n < 255);\n"           /* KO */
              "  memset(buf, 0, n < 255 ? n : 255);\n" /* OK */
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (error) Invalid memset() argument nr 3. A non-boolean value is required.\n", errout.str());

        check("void boolArgZeroIsInvalidButOneIsValid(int param) {\n"
              "  void* buffer = calloc(param > 0, 10);\n"
              "  free(buffer);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (error) Invalid calloc() argument nr 1. The value is 0 or 1 (boolean) but the valid values are '1:'.\n", errout.str());

        check("void boolArgZeroIsValidButOneIsInvalid(int param) {\n"
              "  strtol(a, b, param > 0);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (error) Invalid strtol() argument nr 3. The value is 0 or 1 (boolean) but the valid values are '0,2:36'.\n", errout.str());

        check("int f() { strtol(a,b,1); }");
        ASSERT_EQUALS("[test.cpp:1]: (error) Invalid strtol() argument nr 3. The value is 1 but the valid values are '0,2:36'.\n", errout.str());

        check("int f() { strtol(a,b,10); }");
        ASSERT_EQUALS("", errout.str());
    }

    void invalidFunctionUsageStrings() {
        check("size_t f() { char x = 'x'; return strlen(&x); }");
        ASSERT_EQUALS("[test.cpp:1]: (error) Invalid strlen() argument nr 1. A nul-terminated string is required.\n", errout.str());

        check("size_t f() { return strlen(&x); }");
        ASSERT_EQUALS("", errout.str());

        check("size_t f(char x) { return strlen(&x); }");
        ASSERT_EQUALS("[test.cpp:1]: (error) Invalid strlen() argument nr 1. A nul-terminated string is required.\n", errout.str());

        check("size_t f() { char x = '\\0'; return strlen(&x); }");
        ASSERT_EQUALS("", errout.str());

        check("size_t f() {\n"
              "  char x;\n"
              "  if (y)\n"
              "    x = '\\0';\n"
              "  else\n"
              "    x = 'a';\n"
              "  return strlen(&x);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7]: (error) Invalid strlen() argument nr 1. A nul-terminated string is required.\n", errout.str());

        check("int f() { char x = '\\0'; return strcmp(\"Hello world\", &x); }");
        ASSERT_EQUALS("", errout.str());

        check("int f() { char x = 'x'; return strcmp(\"Hello world\", &x); }");
        ASSERT_EQUALS("[test.cpp:1]: (error) Invalid strcmp() argument nr 2. A nul-terminated string is required.\n", errout.str());

        check("size_t f(char x) { char * y = &x; return strlen(y) }");
        ASSERT_EQUALS("[test.cpp:1]: (error) Invalid strlen() argument nr 1. A nul-terminated string is required.\n", errout.str());

        check("size_t f(char x) { char * y = &x; char *z = y; return strlen(z) }");
        ASSERT_EQUALS("[test.cpp:1]: (error) Invalid strlen() argument nr 1. A nul-terminated string is required.\n", errout.str());

        check("size_t f() { char x = 'x'; char * y = &x; char *z = y; return strlen(z) }");
        ASSERT_EQUALS("[test.cpp:1]: (error) Invalid strlen() argument nr 1. A nul-terminated string is required.\n", errout.str());

        check("size_t f() { char x = '\\0'; char * y = &x; char *z = y; return strlen(z) }");
        ASSERT_EQUALS("", errout.str());

        check("size_t f() { char x[] = \"Hello world\"; return strlen(x) }");
        ASSERT_EQUALS("", errout.str());

        check("size_t f(char x[]) { return strlen(x) }");
        ASSERT_EQUALS("", errout.str());

        check("int f(char x, char y) { return strcmp(&x, &y); }");
        ASSERT_EQUALS("[test.cpp:1]: (error) Invalid strcmp() argument nr 1. A nul-terminated string is required.\n"
                      "[test.cpp:1]: (error) Invalid strcmp() argument nr 2. A nul-terminated string is required.\n", errout.str());

        check("size_t f() { char x[] = \"Hello world\"; return strlen(&x[0]) }");
        ASSERT_EQUALS("", errout.str());

        check("size_t f() { char* x = \"Hello world\"; return strlen(&x[0]) }");
        ASSERT_EQUALS("", errout.str());

        check("struct S {\n"
              "  char x;\n"
              "};\n"
              "size_t f() {\n"
              "  S s1 = {0};\n"
              "  S s2;\n;"
              "  s2.x = 'x';\n"
              "  size_t l1 = strlen(&s1.x);\n"
              "  size_t l2 = strlen(&s2.x);\n"
              "  return l1 + l2;\n"
              "}\n");
        TODO_ASSERT_EQUALS("[test.cpp:9]: (error) Invalid strlen() argument nr 1. A nul-terminated string is required.\n", "", errout.str());

        check("const char x = 'x'; size_t f() { return strlen(&x); }");
        TODO_ASSERT_EQUALS("[test.cpp:1]: (error) Invalid strlen() argument nr 1. A nul-terminated string is required.\n", "", errout.str());

        check("const char x = 'x'; size_t f() { char y = x; return strlen(&y); }");
        ASSERT_EQUALS("[test.cpp:1]: (error) Invalid strlen() argument nr 1. A nul-terminated string is required.\n", errout.str());

        check("const char x = '\\0'; size_t f() { return strlen(&x); }");
        ASSERT_EQUALS("", errout.str());

        check("const char x = '\\0'; size_t f() { char y = x; return strlen(&y); }");
        ASSERT_EQUALS("", errout.str());

        check("size_t f() {\n"
              "  char * a = \"Hello world\";\n"
              "  char ** b = &a;\n"
              "  return strlen(&b[0][0]);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("size_t f() {\n"
              "  char ca[] = \"asdf\";\n"
              "  return strlen((char*) &ca);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        // #5225
        check("int main(void)\n"
              "{\n"
              "  char str[80] = \"hello worl\";\n"
              "  char d = 'd';\n"
              "  strcat(str, &d);\n"
              "  puts(str);\n"
              "  return 0;\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:5]: (error) Invalid strcat() argument nr 2. A nul-terminated string is required.\n", errout.str());
    }

    void mathfunctionCall_sqrt() {
        // sqrt, sqrtf, sqrtl
        check("void foo()\n"
              "{\n"
              "    std::cout <<  sqrt(-1) << std::endl;\n"
              "    std::cout <<  sqrtf(-1) << std::endl;\n"
              "    std::cout <<  sqrtl(-1) << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Invalid sqrt() argument nr 1. The value is -1 but the valid values are '0.0:'.\n"
                      "[test.cpp:4]: (error) Invalid sqrtf() argument nr 1. The value is -1 but the valid values are '0.0:'.\n"
                      "[test.cpp:5]: (error) Invalid sqrtl() argument nr 1. The value is -1 but the valid values are '0.0:'.\n", errout.str());

        // implementation-defined behaviour for "finite values of x<0" only:
        check("void foo()\n"
              "{\n"
              "    std::cout <<  sqrt(-0.) << std::endl;\n"
              "    std::cout <<  sqrtf(-0.) << std::endl;\n"
              "    std::cout <<  sqrtl(-0.) << std::endl;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::cout <<  sqrt(1) << std::endl;\n"
              "    std::cout <<  sqrtf(1) << std::endl;\n"
              "    std::cout <<  sqrtl(1) << std::endl;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void mathfunctionCall_log() {
        // log,log10,logf,logl,log10f,log10l,log2,log2f,log2l,log1p,log1pf,log1pl
        check("void foo()\n"
              "{\n"
              "    std::cout <<  log(-2) << std::endl;\n"
              "    std::cout <<  logf(-2) << std::endl;\n"
              "    std::cout <<  logl(-2) << std::endl;\n"
              "    std::cout <<  log10(-2) << std::endl;\n"
              "    std::cout <<  log10f(-2) << std::endl;\n"
              "    std::cout <<  log10l(-2) << std::endl;\n"
              "    std::cout <<  log2(-2) << std::endl;\n"
              "    std::cout <<  log2f(-2) << std::endl;\n"
              "    std::cout <<  log2l(-2) << std::endl;\n"
              "    std::cout <<  log1p(-3) << std::endl;\n"
              "    std::cout <<  log1pf(-3) << std::endl;\n"
              "    std::cout <<  log1pl(-3) << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Invalid log() argument nr 1. The value is -2 but the valid values are '4.94066e-324:'.\n"
                      "[test.cpp:4]: (error) Invalid logf() argument nr 1. The value is -2 but the valid values are '1.4013e-45:'.\n"
                      "[test.cpp:5]: (error) Invalid logl() argument nr 1. The value is -2 but the valid values are '4.94066e-324:'.\n"
                      "[test.cpp:6]: (error) Invalid log10() argument nr 1. The value is -2 but the valid values are '4.94066e-324:'.\n"
                      "[test.cpp:7]: (error) Invalid log10f() argument nr 1. The value is -2 but the valid values are '1.4013e-45:'.\n"
                      "[test.cpp:8]: (error) Invalid log10l() argument nr 1. The value is -2 but the valid values are '4.94066e-324:'.\n"
                      "[test.cpp:9]: (error) Invalid log2() argument nr 1. The value is -2 but the valid values are '4.94066e-324:'.\n"
                      "[test.cpp:10]: (error) Invalid log2f() argument nr 1. The value is -2 but the valid values are '1.4013e-45:'.\n"
                      "[test.cpp:11]: (error) Invalid log2l() argument nr 1. The value is -2 but the valid values are '4.94066e-324:'.\n"
                      "[test.cpp:3]: (warning) Passing value -2 to log() leads to implementation-defined result.\n"
                      "[test.cpp:4]: (warning) Passing value -2 to logf() leads to implementation-defined result.\n"
                      "[test.cpp:5]: (warning) Passing value -2 to logl() leads to implementation-defined result.\n"
                      "[test.cpp:6]: (warning) Passing value -2 to log10() leads to implementation-defined result.\n"
                      "[test.cpp:7]: (warning) Passing value -2 to log10f() leads to implementation-defined result.\n"
                      "[test.cpp:8]: (warning) Passing value -2 to log10l() leads to implementation-defined result.\n"
                      "[test.cpp:9]: (warning) Passing value -2 to log2() leads to implementation-defined result.\n"
                      "[test.cpp:10]: (warning) Passing value -2 to log2f() leads to implementation-defined result.\n"
                      "[test.cpp:11]: (warning) Passing value -2 to log2l() leads to implementation-defined result.\n"
                      "[test.cpp:12]: (warning) Passing value -3 to log1p() leads to implementation-defined result.\n"
                      "[test.cpp:13]: (warning) Passing value -3 to log1pf() leads to implementation-defined result.\n"
                      "[test.cpp:14]: (warning) Passing value -3 to log1pl() leads to implementation-defined result.\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::cout <<  log(-1) << std::endl;\n"
              "    std::cout <<  logf(-1) << std::endl;\n"
              "    std::cout <<  logl(-1) << std::endl;\n"
              "    std::cout <<  log10(-1) << std::endl;\n"
              "    std::cout <<  log10f(-1) << std::endl;\n"
              "    std::cout <<  log10l(-1) << std::endl;\n"
              "    std::cout <<  log2(-1) << std::endl;\n"
              "    std::cout <<  log2f(-1) << std::endl;\n"
              "    std::cout <<  log2l(-1) << std::endl;\n"
              "    std::cout <<  log1p(-2) << std::endl;\n"
              "    std::cout <<  log1pf(-2) << std::endl;\n"
              "    std::cout <<  log1pl(-2) << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Invalid log() argument nr 1. The value is -1 but the valid values are '4.94066e-324:'.\n"
                      "[test.cpp:4]: (error) Invalid logf() argument nr 1. The value is -1 but the valid values are '1.4013e-45:'.\n"
                      "[test.cpp:5]: (error) Invalid logl() argument nr 1. The value is -1 but the valid values are '4.94066e-324:'.\n"
                      "[test.cpp:6]: (error) Invalid log10() argument nr 1. The value is -1 but the valid values are '4.94066e-324:'.\n"
                      "[test.cpp:7]: (error) Invalid log10f() argument nr 1. The value is -1 but the valid values are '1.4013e-45:'.\n"
                      "[test.cpp:8]: (error) Invalid log10l() argument nr 1. The value is -1 but the valid values are '4.94066e-324:'.\n"
                      "[test.cpp:9]: (error) Invalid log2() argument nr 1. The value is -1 but the valid values are '4.94066e-324:'.\n"
                      "[test.cpp:10]: (error) Invalid log2f() argument nr 1. The value is -1 but the valid values are '1.4013e-45:'.\n"
                      "[test.cpp:11]: (error) Invalid log2l() argument nr 1. The value is -1 but the valid values are '4.94066e-324:'.\n"
                      "[test.cpp:3]: (warning) Passing value -1 to log() leads to implementation-defined result.\n"
                      "[test.cpp:4]: (warning) Passing value -1 to logf() leads to implementation-defined result.\n"
                      "[test.cpp:5]: (warning) Passing value -1 to logl() leads to implementation-defined result.\n"
                      "[test.cpp:6]: (warning) Passing value -1 to log10() leads to implementation-defined result.\n"
                      "[test.cpp:7]: (warning) Passing value -1 to log10f() leads to implementation-defined result.\n"
                      "[test.cpp:8]: (warning) Passing value -1 to log10l() leads to implementation-defined result.\n"
                      "[test.cpp:9]: (warning) Passing value -1 to log2() leads to implementation-defined result.\n"
                      "[test.cpp:10]: (warning) Passing value -1 to log2f() leads to implementation-defined result.\n"
                      "[test.cpp:11]: (warning) Passing value -1 to log2l() leads to implementation-defined result.\n"
                      "[test.cpp:12]: (warning) Passing value -2 to log1p() leads to implementation-defined result.\n"
                      "[test.cpp:13]: (warning) Passing value -2 to log1pf() leads to implementation-defined result.\n"
                      "[test.cpp:14]: (warning) Passing value -2 to log1pl() leads to implementation-defined result.\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::cout <<  log(-1.0) << std::endl;\n"
              "    std::cout <<  logf(-1.0) << std::endl;\n"
              "    std::cout <<  logl(-1.0) << std::endl;\n"
              "    std::cout <<  log10(-1.0) << std::endl;\n"
              "    std::cout <<  log10f(-1.0) << std::endl;\n"
              "    std::cout <<  log10l(-1.0) << std::endl;\n"
              "    std::cout <<  log2(-1.0) << std::endl;\n"
              "    std::cout <<  log2f(-1.0) << std::endl;\n"
              "    std::cout <<  log2l(-1.0) << std::endl;\n"
              "    std::cout <<  log1p(-2.0) << std::endl;\n"
              "    std::cout <<  log1pf(-2.0) << std::endl;\n"
              "    std::cout <<  log1pl(-2.0) << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Invalid log() argument nr 1. The value is -1 but the valid values are '4.94066e-324:'.\n"
                      "[test.cpp:4]: (error) Invalid logf() argument nr 1. The value is -1 but the valid values are '1.4013e-45:'.\n"
                      "[test.cpp:5]: (error) Invalid logl() argument nr 1. The value is -1 but the valid values are '4.94066e-324:'.\n"
                      "[test.cpp:6]: (error) Invalid log10() argument nr 1. The value is -1 but the valid values are '4.94066e-324:'.\n"
                      "[test.cpp:7]: (error) Invalid log10f() argument nr 1. The value is -1 but the valid values are '1.4013e-45:'.\n"
                      "[test.cpp:8]: (error) Invalid log10l() argument nr 1. The value is -1 but the valid values are '4.94066e-324:'.\n"
                      "[test.cpp:9]: (error) Invalid log2() argument nr 1. The value is -1 but the valid values are '4.94066e-324:'.\n"
                      "[test.cpp:10]: (error) Invalid log2f() argument nr 1. The value is -1 but the valid values are '1.4013e-45:'.\n"
                      "[test.cpp:11]: (error) Invalid log2l() argument nr 1. The value is -1 but the valid values are '4.94066e-324:'.\n"
                      "[test.cpp:3]: (warning) Passing value -1.0 to log() leads to implementation-defined result.\n"
                      "[test.cpp:4]: (warning) Passing value -1.0 to logf() leads to implementation-defined result.\n"
                      "[test.cpp:5]: (warning) Passing value -1.0 to logl() leads to implementation-defined result.\n"
                      "[test.cpp:6]: (warning) Passing value -1.0 to log10() leads to implementation-defined result.\n"
                      "[test.cpp:7]: (warning) Passing value -1.0 to log10f() leads to implementation-defined result.\n"
                      "[test.cpp:8]: (warning) Passing value -1.0 to log10l() leads to implementation-defined result.\n"
                      "[test.cpp:9]: (warning) Passing value -1.0 to log2() leads to implementation-defined result.\n"
                      "[test.cpp:10]: (warning) Passing value -1.0 to log2f() leads to implementation-defined result.\n"
                      "[test.cpp:11]: (warning) Passing value -1.0 to log2l() leads to implementation-defined result.\n"
                      "[test.cpp:12]: (warning) Passing value -2.0 to log1p() leads to implementation-defined result.\n"
                      "[test.cpp:13]: (warning) Passing value -2.0 to log1pf() leads to implementation-defined result.\n"
                      "[test.cpp:14]: (warning) Passing value -2.0 to log1pl() leads to implementation-defined result.\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::cout <<  log(-0.1) << std::endl;\n"
              "    std::cout <<  logf(-0.1) << std::endl;\n"
              "    std::cout <<  logl(-0.1) << std::endl;\n"
              "    std::cout <<  log10(-0.1) << std::endl;\n"
              "    std::cout <<  log10f(-0.1) << std::endl;\n"
              "    std::cout <<  log10l(-0.1) << std::endl;\n"
              "    std::cout <<  log2(-0.1) << std::endl;\n"
              "    std::cout <<  log2f(-0.1) << std::endl;\n"
              "    std::cout <<  log2l(-0.1) << std::endl;\n"
              "    std::cout <<  log1p(-1.1) << std::endl;\n"
              "    std::cout <<  log1pf(-1.1) << std::endl;\n"
              "    std::cout <<  log1pl(-1.1) << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Invalid log() argument nr 1. The value is -0.1 but the valid values are '4.94066e-324:'.\n"
                      "[test.cpp:4]: (error) Invalid logf() argument nr 1. The value is -0.1 but the valid values are '1.4013e-45:'.\n"
                      "[test.cpp:5]: (error) Invalid logl() argument nr 1. The value is -0.1 but the valid values are '4.94066e-324:'.\n"
                      "[test.cpp:6]: (error) Invalid log10() argument nr 1. The value is -0.1 but the valid values are '4.94066e-324:'.\n"
                      "[test.cpp:7]: (error) Invalid log10f() argument nr 1. The value is -0.1 but the valid values are '1.4013e-45:'.\n"
                      "[test.cpp:8]: (error) Invalid log10l() argument nr 1. The value is -0.1 but the valid values are '4.94066e-324:'.\n"
                      "[test.cpp:9]: (error) Invalid log2() argument nr 1. The value is -0.1 but the valid values are '4.94066e-324:'.\n"
                      "[test.cpp:10]: (error) Invalid log2f() argument nr 1. The value is -0.1 but the valid values are '1.4013e-45:'.\n"
                      "[test.cpp:11]: (error) Invalid log2l() argument nr 1. The value is -0.1 but the valid values are '4.94066e-324:'.\n"
                      "[test.cpp:3]: (warning) Passing value -0.1 to log() leads to implementation-defined result.\n"
                      "[test.cpp:4]: (warning) Passing value -0.1 to logf() leads to implementation-defined result.\n"
                      "[test.cpp:5]: (warning) Passing value -0.1 to logl() leads to implementation-defined result.\n"
                      "[test.cpp:6]: (warning) Passing value -0.1 to log10() leads to implementation-defined result.\n"
                      "[test.cpp:7]: (warning) Passing value -0.1 to log10f() leads to implementation-defined result.\n"
                      "[test.cpp:8]: (warning) Passing value -0.1 to log10l() leads to implementation-defined result.\n"
                      "[test.cpp:9]: (warning) Passing value -0.1 to log2() leads to implementation-defined result.\n"
                      "[test.cpp:10]: (warning) Passing value -0.1 to log2f() leads to implementation-defined result.\n"
                      "[test.cpp:11]: (warning) Passing value -0.1 to log2l() leads to implementation-defined result.\n"
                      "[test.cpp:12]: (warning) Passing value -1.1 to log1p() leads to implementation-defined result.\n"
                      "[test.cpp:13]: (warning) Passing value -1.1 to log1pf() leads to implementation-defined result.\n"
                      "[test.cpp:14]: (warning) Passing value -1.1 to log1pl() leads to implementation-defined result.\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::cout <<  log(0) << std::endl;\n"
              "    std::cout <<  logf(0.) << std::endl;\n"
              "    std::cout <<  logl(0.0) << std::endl;\n"
              "    std::cout <<  log10(0.0) << std::endl;\n"
              "    std::cout <<  log10f(0) << std::endl;\n"
              "    std::cout <<  log10l(0.) << std::endl;\n"
              "    std::cout <<  log2(0.) << std::endl;\n"
              "    std::cout <<  log2f(0.0) << std::endl;\n"
              "    std::cout <<  log2l(0) << std::endl;\n"
              "    std::cout <<  log1p(-1.) << std::endl;\n"
              "    std::cout <<  log1pf(-1.0) << std::endl;\n"
              "    std::cout <<  log1pl(-1) << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Invalid log() argument nr 1. The value is 0 but the valid values are '4.94066e-324:'.\n"
                      "[test.cpp:4]: (error) Invalid logf() argument nr 1. The value is 0 but the valid values are '1.4013e-45:'.\n"
                      "[test.cpp:5]: (error) Invalid logl() argument nr 1. The value is 0 but the valid values are '4.94066e-324:'.\n"
                      "[test.cpp:6]: (error) Invalid log10() argument nr 1. The value is 0 but the valid values are '4.94066e-324:'.\n"
                      "[test.cpp:7]: (error) Invalid log10f() argument nr 1. The value is 0 but the valid values are '1.4013e-45:'.\n"
                      "[test.cpp:8]: (error) Invalid log10l() argument nr 1. The value is 0 but the valid values are '4.94066e-324:'.\n"
                      "[test.cpp:9]: (error) Invalid log2() argument nr 1. The value is 0 but the valid values are '4.94066e-324:'.\n"
                      "[test.cpp:10]: (error) Invalid log2f() argument nr 1. The value is 0 but the valid values are '1.4013e-45:'.\n"
                      "[test.cpp:11]: (error) Invalid log2l() argument nr 1. The value is 0 but the valid values are '4.94066e-324:'.\n"
                      "[test.cpp:3]: (warning) Passing value 0 to log() leads to implementation-defined result.\n"
                      "[test.cpp:4]: (warning) Passing value 0. to logf() leads to implementation-defined result.\n"
                      "[test.cpp:5]: (warning) Passing value 0.0 to logl() leads to implementation-defined result.\n"
                      "[test.cpp:6]: (warning) Passing value 0.0 to log10() leads to implementation-defined result.\n"
                      "[test.cpp:7]: (warning) Passing value 0 to log10f() leads to implementation-defined result.\n"
                      "[test.cpp:8]: (warning) Passing value 0. to log10l() leads to implementation-defined result.\n"
                      "[test.cpp:9]: (warning) Passing value 0. to log2() leads to implementation-defined result.\n"
                      "[test.cpp:10]: (warning) Passing value 0.0 to log2f() leads to implementation-defined result.\n"
                      "[test.cpp:11]: (warning) Passing value 0 to log2l() leads to implementation-defined result.\n"
                      "[test.cpp:12]: (warning) Passing value -1. to log1p() leads to implementation-defined result.\n"
                      "[test.cpp:13]: (warning) Passing value -1.0 to log1pf() leads to implementation-defined result.\n"
                      "[test.cpp:14]: (warning) Passing value -1 to log1pl() leads to implementation-defined result.\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::cout <<  log(1E-3)         << std::endl;\n"
              "    std::cout <<  logf(1E-3)        << std::endl;\n"
              "    std::cout <<  logl(1E-3)        << std::endl;\n"
              "    std::cout <<  log10(1E-3)       << std::endl;\n"
              "    std::cout <<  log10f(1E-3)      << std::endl;\n"
              "    std::cout <<  log10l(1E-3)      << std::endl;\n"
              "    std::cout <<  log2(1E-3)        << std::endl;\n"
              "    std::cout <<  log2f(1E-3)       << std::endl;\n"
              "    std::cout <<  log2l(1E-3)       << std::endl;\n"
              "    std::cout <<  log1p(-1+1E-3)    << std::endl;\n"
              "    std::cout <<  log1pf(-1+1E-3)   << std::endl;\n"
              "    std::cout <<  log1pl(-1+1E-3)   << std::endl;\n"
              "    std::cout <<  log(1.0E-3)       << std::endl;\n"
              "    std::cout <<  logf(1.0E-3)      << std::endl;\n"
              "    std::cout <<  logl(1.0E-3)      << std::endl;\n"
              "    std::cout <<  log10(1.0E-3)     << std::endl;\n"
              "    std::cout <<  log10f(1.0E-3)    << std::endl;\n"
              "    std::cout <<  log10l(1.0E-3)    << std::endl;\n"
              "    std::cout <<  log2(1.0E-3)      << std::endl;\n"
              "    std::cout <<  log2f(1.0E-3)     << std::endl;\n"
              "    std::cout <<  log2l(1.0E-3)     << std::endl;\n"
              "    std::cout <<  log1p(-1+1.0E-3)  << std::endl;\n"
              "    std::cout <<  log1pf(-1+1.0E-3) << std::endl;\n"
              "    std::cout <<  log1pl(-1+1.0E-3) << std::endl;\n"
              "    std::cout <<  log(1.0E+3)       << std::endl;\n"
              "    std::cout <<  logf(1.0E+3)      << std::endl;\n"
              "    std::cout <<  logl(1.0E+3)      << std::endl;\n"
              "    std::cout <<  log10(1.0E+3)     << std::endl;\n"
              "    std::cout <<  log10f(1.0E+3)    << std::endl;\n"
              "    std::cout <<  log10l(1.0E+3)    << std::endl;\n"
              "    std::cout <<  log2(1.0E+3)      << std::endl;\n"
              "    std::cout <<  log2f(1.0E+3)     << std::endl;\n"
              "    std::cout <<  log2l(1.0E+3)     << std::endl;\n"
              "    std::cout <<  log1p(1.0E+3)     << std::endl;\n"
              "    std::cout <<  log1pf(1.0E+3)    << std::endl;\n"
              "    std::cout <<  log1pl(1.0E+3)    << std::endl;\n"
              "    std::cout <<  log(2.0)          << std::endl;\n"
              "    std::cout <<  logf(2.0)         << std::endl;\n"
              "    std::cout <<  logf(2.0f)        << std::endl;\n"
              "    std::cout <<  log10(2.0)        << std::endl;\n"
              "    std::cout <<  log10f(2.0)       << std::endl;\n"
              "    std::cout <<  log10f(2.0f)      << std::endl;\n"
              "    std::cout <<  log2(2.0)         << std::endl;\n"
              "    std::cout <<  log2f(2.0)        << std::endl;\n"
              "    std::cout <<  log2f(2.0f)       << std::endl;\n"
              "    std::cout <<  log1p(2.0)        << std::endl;\n"
              "    std::cout <<  log1pf(2.0)       << std::endl;\n"
              "    std::cout <<  log1pf(2.0f)      << std::endl;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::string *log(0);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #3473 - no warning if "log" is a variable
        check("Fred::Fred() : log(0) { }");
        ASSERT_EQUALS("", errout.str());

        // #5748
        check("void f() { foo.log(0); }");
        ASSERT_EQUALS("", errout.str());
    }

    void mathfunctionCall_acos() {
        // acos, acosf, acosl
        check("void foo()\n"
              "{\n"
              " return acos(-1)      \n"
              "    + acos(0.1)       \n"
              "    + acos(0.0001)    \n"
              "    + acos(0.01)      \n"
              "    + acos(1.0E-1)    \n"
              "    + acos(-1.0E-1)   \n"
              "    + acos(+1.0E-1)   \n"
              "    + acos(0.1E-1)    \n"
              "    + acos(+0.1E-1)   \n"
              "    + acos(-0.1E-1)   \n"
              "    + acosf(-1)       \n"
              "    + acosf(0.1)      \n"
              "    + acosf(0.0001)   \n"
              "    + acosf(0.01)     \n"
              "    + acosf(1.0E-1)   \n"
              "    + acosf(-1.0E-1)  \n"
              "    + acosf(+1.0E-1)  \n"
              "    + acosf(0.1E-1)   \n"
              "    + acosf(+0.1E-1)  \n"
              "    + acosf(-0.1E-1)  \n"
              "    + acosl(-1)       \n"
              "    + acosl(0.1)      \n"
              "    + acosl(0.0001)   \n"
              "    + acosl(0.01)     \n"
              "    + acosl(1.0E-1)   \n"
              "    + acosl(-1.0E-1)  \n"
              "    + acosl(+1.0E-1)  \n"
              "    + acosl(0.1E-1)   \n"
              "    + acosl(+0.1E-1)  \n"
              "    + acosl(-0.1E-1); \n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::cout <<  acos(1.1) << std::endl;\n"
              "    std::cout <<  acosf(1.1) << std::endl;\n"
              "    std::cout <<  acosl(1.1) << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Invalid acos() argument nr 1. The value is 1.1 but the valid values are '-1.0:1.0'.\n"
                      "[test.cpp:4]: (error) Invalid acosf() argument nr 1. The value is 1.1 but the valid values are '-1.0:1.0'.\n"
                      "[test.cpp:5]: (error) Invalid acosl() argument nr 1. The value is 1.1 but the valid values are '-1.0:1.0'.\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::cout <<  acos(-1.1) << std::endl;\n"
              "    std::cout <<  acosf(-1.1) << std::endl;\n"
              "    std::cout <<  acosl(-1.1) << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Invalid acos() argument nr 1. The value is -1.1 but the valid values are '-1.0:1.0'.\n"
                      "[test.cpp:4]: (error) Invalid acosf() argument nr 1. The value is -1.1 but the valid values are '-1.0:1.0'.\n"
                      "[test.cpp:5]: (error) Invalid acosl() argument nr 1. The value is -1.1 but the valid values are '-1.0:1.0'.\n", errout.str());
    }

    void mathfunctionCall_asin() {
        // asin, asinf, asinl
        check("void foo()\n"
              "{\n"
              " return asin(1)       \n"
              "    + asin(-1)        \n"
              "    + asin(0.1)       \n"
              "    + asin(0.0001)    \n"
              "    + asin(0.01)      \n"
              "    + asin(1.0E-1)    \n"
              "    + asin(-1.0E-1)   \n"
              "    + asin(+1.0E-1)   \n"
              "    + asin(0.1E-1)    \n"
              "    + asin(+0.1E-1)   \n"
              "    + asin(-0.1E-1)   \n"
              "    + asinf(1)        \n"
              "    + asinf(-1)       \n"
              "    + asinf(0.1)      \n"
              "    + asinf(0.0001)   \n"
              "    + asinf(0.01)     \n"
              "    + asinf(1.0E-1)   \n"
              "    + asinf(-1.0E-1)  \n"
              "    + asinf(+1.0E-1)  \n"
              "    + asinf(0.1E-1)   \n"
              "    + asinf(+0.1E-1)  \n"
              "    + asinf(-0.1E-1)  \n"
              "    + asinl(1)        \n"
              "    + asinl(-1)       \n"
              "    + asinl(0.1)      \n"
              "    + asinl(0.0001)   \n"
              "    + asinl(0.01)     \n"
              "    + asinl(1.0E-1)   \n"
              "    + asinl(-1.0E-1)  \n"
              "    + asinl(+1.0E-1)  \n"
              "    + asinl(0.1E-1)   \n"
              "    + asinl(+0.1E-1)  \n"
              "    + asinl(-0.1E-1); \n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::cout <<  asin(1.1) << std::endl;\n"
              "    std::cout <<  asinf(1.1) << std::endl;\n"
              "    std::cout <<  asinl(1.1) << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Invalid asin() argument nr 1. The value is 1.1 but the valid values are '-1.0:1.0'.\n"
                      "[test.cpp:4]: (error) Invalid asinf() argument nr 1. The value is 1.1 but the valid values are '-1.0:1.0'.\n"
                      "[test.cpp:5]: (error) Invalid asinl() argument nr 1. The value is 1.1 but the valid values are '-1.0:1.0'.\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::cout <<  asin(-1.1) << std::endl;\n"
              "    std::cout <<  asinf(-1.1) << std::endl;\n"
              "    std::cout <<  asinl(-1.1) << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Invalid asin() argument nr 1. The value is -1.1 but the valid values are '-1.0:1.0'.\n"
                      "[test.cpp:4]: (error) Invalid asinf() argument nr 1. The value is -1.1 but the valid values are '-1.0:1.0'.\n"
                      "[test.cpp:5]: (error) Invalid asinl() argument nr 1. The value is -1.1 but the valid values are '-1.0:1.0'.\n", errout.str());
    }

    void mathfunctionCall_pow() {
        // pow, powf, powl
        check("void foo()\n"
              "{\n"
              "    std::cout <<  pow(0,-10) << std::endl;\n"
              "    std::cout <<  powf(0,-10) << std::endl;\n"
              "    std::cout <<  powl(0,-10) << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Passing values 0 and -10 to pow() leads to implementation-defined result.\n"
                      "[test.cpp:4]: (warning) Passing values 0 and -10 to powf() leads to implementation-defined result.\n"
                      "[test.cpp:5]: (warning) Passing values 0 and -10 to powl() leads to implementation-defined result.\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::cout <<  pow(0,10) << std::endl;\n"
              "    std::cout <<  powf(0,10) << std::endl;\n"
              "    std::cout <<  powl(0,10) << std::endl;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void mathfunctionCall_atan2() {
        // atan2
        check("void foo()\n"
              "{\n"
              "    std::cout <<  atan2(1,1)         ;\n"
              "    std::cout <<  atan2(-1,-1)       ;\n"
              "    std::cout <<  atan2(0.1,1)       ;\n"
              "    std::cout <<  atan2(0.0001,100)  ;\n"
              "    std::cout <<  atan2(0.0,1e-1)    ;\n"
              "    std::cout <<  atan2(1.0E-1,-3)   ;\n"
              "    std::cout <<  atan2(-1.0E-1,+2)  ;\n"
              "    std::cout <<  atan2(+1.0E-1,0)   ;\n"
              "    std::cout <<  atan2(0.1E-1,3)    ;\n"
              "    std::cout <<  atan2(+0.1E-1,1)   ;\n"
              "    std::cout <<  atan2(-0.1E-1,8)   ;\n"
              "    std::cout <<  atan2f(1,1)        ;\n"
              "    std::cout <<  atan2f(-1,-1)      ;\n"
              "    std::cout <<  atan2f(0.1,1)      ;\n"
              "    std::cout <<  atan2f(0.0001,100) ;\n"
              "    std::cout <<  atan2f(0.0,1e-1)   ;\n"
              "    std::cout <<  atan2f(1.0E-1,-3)  ;\n"
              "    std::cout <<  atan2f(-1.0E-1,+2) ;\n"
              "    std::cout <<  atan2f(+1.0E-1,0)  ;\n"
              "    std::cout <<  atan2f(0.1E-1,3)   ;\n"
              "    std::cout <<  atan2f(+0.1E-1,1)  ;\n"
              "    std::cout <<  atan2f(-0.1E-1,8)  ;\n"
              "    std::cout <<  atan2l(1,1)        ;\n"
              "    std::cout <<  atan2l(-1,-1)      ;\n"
              "    std::cout <<  atan2l(0.1,1)      ;\n"
              "    std::cout <<  atan2l(0.0001,100) ;\n"
              "    std::cout <<  atan2l(0.0,1e-1)   ;\n"
              "    std::cout <<  atan2l(1.0E-1,-3)  ;\n"
              "    std::cout <<  atan2l(-1.0E-1,+2) ;\n"
              "    std::cout <<  atan2l(+1.0E-1,0)  ;\n"
              "    std::cout <<  atan2l(0.1E-1,3)   ;\n"
              "    std::cout <<  atan2l(+0.1E-1,1)  ;\n"
              "    std::cout <<  atan2l(-0.1E-1,8)  ;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::cout <<  atan2(0,0) << std::endl;\n"
              "    std::cout <<  atan2f(0,0) << std::endl;\n"
              "    std::cout <<  atan2l(0,0) << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Passing values 0 and 0 to atan2() leads to implementation-defined result.\n"
                      "[test.cpp:4]: (warning) Passing values 0 and 0 to atan2f() leads to implementation-defined result.\n"
                      "[test.cpp:5]: (warning) Passing values 0 and 0 to atan2l() leads to implementation-defined result.\n", errout.str());
    }

    void mathfunctionCall_fmod() {
        // fmod, fmodl, fmodf
        check("void foo()\n"
              "{\n"
              "    std::cout <<  fmod(1.0,0) << std::endl;\n"
              "    std::cout <<  fmodf(1.0,0) << std::endl;\n"
              "    std::cout <<  fmodl(1.0,0) << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Passing values 1.0 and 0 to fmod() leads to implementation-defined result.\n"
                      "[test.cpp:4]: (warning) Passing values 1.0 and 0 to fmodf() leads to implementation-defined result.\n"
                      "[test.cpp:5]: (warning) Passing values 1.0 and 0 to fmodl() leads to implementation-defined result.\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::cout <<  fmod(1.0,1) << std::endl;\n"
              "    std::cout <<  fmodf(1.0,1) << std::endl;\n"
              "    std::cout <<  fmodl(1.0,1) << std::endl;\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void mathfunctionCall_precision() {
        check("void foo() {\n"
              "    print(exp(x) - 1);\n"
              "    print(log(1 + x));\n"
              "    print(1 - erf(x));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Expression 'exp(x) - 1' can be replaced by 'expm1(x)' to avoid loss of precision.\n"
                      "[test.cpp:3]: (style) Expression 'log(1 + x)' can be replaced by 'log1p(x)' to avoid loss of precision.\n"
                      "[test.cpp:4]: (style) Expression '1 - erf(x)' can be replaced by 'erfc(x)' to avoid loss of precision.\n", errout.str());

        check("void foo() {\n"
              "    print(exp(x) - 1.0);\n"
              "    print(log(1.0 + x));\n"
              "    print(1.0 - erf(x));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Expression 'exp(x) - 1' can be replaced by 'expm1(x)' to avoid loss of precision.\n"
                      "[test.cpp:3]: (style) Expression 'log(1 + x)' can be replaced by 'log1p(x)' to avoid loss of precision.\n"
                      "[test.cpp:4]: (style) Expression '1 - erf(x)' can be replaced by 'erfc(x)' to avoid loss of precision.\n", errout.str());

        check("void foo() {\n"
              "    print(exp(3 + x*f(a)) - 1);\n"
              "    print(log(x*4 + 1));\n"
              "    print(1 - erf(34*x + f(x) - c));\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (style) Expression 'exp(x) - 1' can be replaced by 'expm1(x)' to avoid loss of precision.\n"
                      "[test.cpp:3]: (style) Expression 'log(1 + x)' can be replaced by 'log1p(x)' to avoid loss of precision.\n"
                      "[test.cpp:4]: (style) Expression '1 - erf(x)' can be replaced by 'erfc(x)' to avoid loss of precision.\n", errout.str());

        check("void foo() {\n"
              "    print(2*exp(x) - 1);\n"
              "    print(1 - erf(x)/2.0);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void checkIgnoredReturnValue() {
        Settings settings2;
        settings2.addEnabled("warning");
        const char xmldata[] = "<?xml version=\"1.0\"?>\n"
                               "<def version=\"2\">\n"
                               "  <function name=\"mystrcmp,foo::mystrcmp\">\n"
                               "    <use-retval/>\n"
                               "    <arg nr=\"1\"/>\n"
                               "    <arg nr=\"2\"/>\n"
                               "  </function>\n"
                               "</def>";
        tinyxml2::XMLDocument doc;
        doc.Parse(xmldata, sizeof(xmldata));
        settings2.library.load(doc);

        check("void foo() {\n"
              "  mystrcmp(a, b);\n"
              "}", "test.cpp", &settings2);
        ASSERT_EQUALS("[test.cpp:2]: (warning) Return value of function mystrcmp() is not used.\n", errout.str());

        check("void foo() {\n"
              "  foo::mystrcmp(a, b);\n"
              "}", "test.cpp", &settings2);
        ASSERT_EQUALS("[test.cpp:2]: (warning) Return value of function foo::mystrcmp() is not used.\n", errout.str());

        check("void f() {\n"
              "  foo x;\n"
              "  x.mystrcmp(a, b);\n"
              "}", "test.cpp", &settings2);
        ASSERT_EQUALS("[test.cpp:3]: (warning) Return value of function x.mystrcmp() is not used.\n", errout.str());

        check("bool mystrcmp(char* a, char* b);\n" // cppcheck sees a custom strcmp definition, but it returns a value. Assume it is the one specified in the library.
              "void foo() {\n"
              "    mystrcmp(a, b);\n"
              "}", "test.cpp", &settings2);
        ASSERT_EQUALS("[test.cpp:3]: (warning) Return value of function mystrcmp() is not used.\n", errout.str());

        check("void mystrcmp(char* a, char* b);\n" // cppcheck sees a custom strcmp definition which returns void!
              "void foo() {\n"
              "    mystrcmp(a, b);\n"
              "}", "test.cpp", &settings2);
        ASSERT_EQUALS("", errout.str());

        check("void foo() {\n"
              "    class mystrcmp { mystrcmp() {} };\n" // strcmp is a constructor definition here
              "}", "test.cpp", &settings2);
        ASSERT_EQUALS("", errout.str());

        check("void foo() {\n"
              "    return mystrcmp(a, b);\n"
              "}", "test.cpp", &settings2);
        ASSERT_EQUALS("", errout.str());

        check("void foo() {\n"
              "    return foo::mystrcmp(a, b);\n"
              "}", "test.cpp", &settings2);
        ASSERT_EQUALS("", errout.str());

        check("void foo() {\n"
              "    if(mystrcmp(a, b));\n"
              "}", "test.cpp", &settings2);
        ASSERT_EQUALS("", errout.str());

        check("void foo() {\n"
              "    bool b = mystrcmp(a, b);\n"
              "}", "test.cpp", &settings2);
        ASSERT_EQUALS("", errout.str());

        // #6194
        check("void foo() {\n"
              "    MyStrCmp mystrcmp(x, y);\n"
              "}", "test.cpp", &settings2);
        ASSERT_EQUALS("", errout.str());

        // #6197
        check("void foo() {\n"
              "    abc::def.mystrcmp(a,b);\n"
              "}", "test.cpp", &settings2);
        ASSERT_EQUALS("", errout.str());

        // #6233
        check("int main() {\n"
              "    auto lambda = [](double value) {\n"
              "        double rounded = floor(value + 0.5);\n"
              "        printf(\"Rounded value = %f\\n\", rounded);\n"
              "    };\n"
              "    lambda(13.3);\n"
              "    return 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #6669
        check("void foo(size_t size) {\n"
              "   void * res{malloc(size)};\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #7447
        check("void foo() {\n"
              "   int x{mystrcmp(a,b)};\n"
              "}", "test.cpp", &settings2);
        ASSERT_EQUALS("", errout.str());

        // #7905
        check("void foo() {\n"
              "   int x({mystrcmp(a,b)});\n"
              "}", "test.cpp", &settings2);
        ASSERT_EQUALS("", errout.str());

        check("void foo() {\n" // don't crash
              "  DEBUG(123)(mystrcmp(a,b))(fd);\n"
              "}", "test.c", &settings2);
        check("struct teststruct {\n"
              "    int testfunc1() __attribute__ ((warn_unused_result)) { return 1; }\n"
              "    [[nodiscard]] int testfunc2() { return 1; }\n"
              "    void foo() { testfunc1(); testfunc2(); }\n"
              "};\n"
              "int main() {\n"
              "    teststruct TestStruct1;\n"
              "    TestStruct1.testfunc1();\n"
              "    TestStruct1.testfunc2();\n"
              "}", "test.cpp", &settings2);
        ASSERT_EQUALS("[test.cpp:4]: (warning) Return value of function testfunc1() is not used.\n"
                      "[test.cpp:4]: (warning) Return value of function testfunc2() is not used.\n"
                      "[test.cpp:8]: (warning) Return value of function TestStruct1.testfunc1() is not used.\n"
                      "[test.cpp:9]: (warning) Return value of function TestStruct1.testfunc2() is not used.\n", errout.str());

        // #9006
        check("template <typename... a> uint8_t b(std::tuple<uint8_t> d) {\n"
              "  std::tuple<a...> c{std::move(d)};\n"
              "  return std::get<0>(c);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("struct A { int x; };\n"
              "template <class... Ts>\n"
              "A f(int x, Ts... xs) {\n"
              "    return {std::move(x), static_cast<int>(xs)...};\n"
              "}\n"
              "A g() { return f(1); }\n");
        ASSERT_EQUALS("", errout.str());
    }

    void checkIgnoredErrorCode() {
        Settings settings2;
        settings2.addEnabled("style");
        const char xmldata[] = "<?xml version=\"1.0\"?>\n"
                               "<def version=\"2\">\n"
                               "  <function name=\"mystrcmp\">\n"
                               "    <use-retval type=\"error-code\"/>\n"
                               "    <arg nr=\"1\"/>\n"
                               "    <arg nr=\"2\"/>\n"
                               "  </function>\n"
                               "</def>";
        tinyxml2::XMLDocument doc;
        doc.Parse(xmldata, sizeof(xmldata));
        settings2.library.load(doc);

        check("void foo() {\n"
              "  mystrcmp(a, b);\n"
              "}", "test.cpp", &settings2);
        ASSERT_EQUALS("[test.cpp:2]: (style) Error code from the return value of function mystrcmp() is not used.\n", errout.str());
    }

    void memsetZeroBytes() {
        check("void f() {\n"
              "    memset(p, 10, 0x0);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2]: (warning) memset() called to fill 0 bytes.\n", errout.str());

        check("void f() {\n"
              "    memset(p, sizeof(p), 0);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2]: (warning) memset() called to fill 0 bytes.\n", errout.str());

        check("void f() {\n"
              "    memset(p, sizeof(p), i);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        // #6269 false positives in case of overloaded standard library functions
        check("class c {\n"
              "  void memset( int i );\n"
              "  void f( void )   {\n"
              "     memset( 0 );\n"
              "  }\n"
              "};");
        ASSERT_EQUALS("", errout.str());

        // #7285
        check("void f() {\n"
              "    memset(&tm, sizeof(tm), 0);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2]: (warning) memset() called to fill 0 bytes.\n", errout.str());

    }

    void memsetInvalid2ndParam() {
        check("void f() {\n"
              "    int* is = new int[10];\n"
              "    memset(is, 1.0f, 40);\n"
              "    int* is2 = new int[10];\n"
              "    memset(is2, 0.1f, 40);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (portability) The 2nd memset() argument '1.0f' is a float, its representation is implementation defined.\n"
                      "[test.cpp:5]: (portability) The 2nd memset() argument '0.1f' is a float, its representation is implementation defined.\n", errout.str());

        check("void f() {\n"
              "    int* is = new int[10];\n"
              "    float g = computeG();\n"
              "    memset(is, g, 40);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (portability) The 2nd memset() argument 'g' is a float, its representation is implementation defined.\n", errout.str());

        check("void f() {\n"
              "    int* is = new int[10];\n"
              "    memset(is, 0.0f, 40);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n" // FP
              "    float x = 2.3f;\n"
              "    memset(a, (x?64:0), 40);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    short ss[] = {1, 2};\n"
              "    memset(ss, 256, 4);\n"
              "    short ss2[2];\n"
              "    memset(ss2, -129, 4);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (warning) The 2nd memset() argument '256' doesn't fit into an 'unsigned char'.\n"
                      "[test.cpp:5]: (warning) The 2nd memset() argument '-129' doesn't fit into an 'unsigned char'.\n", errout.str());

        check("void f() {\n"
              "    int is[10];\n"
              "    memset(is, 0xEE, 40);\n"
              "    unsigned char* cs = malloc(256);\n"
              "    memset(cs, -1, 256);\n"
              "    short* ss[30];\n"
              "    memset(ss, -128, 60);\n"
              "    char cs2[30];\n"
              "    memset(cs2, 255, 30);\n"
              "    char cs3[30];\n"
              "    memset(cs3, 0, 30);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void f() {\n"
              "    int is[10];\n"
              "    const int i = g();\n"
              "    memset(is, 1.0f + i, 40);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (portability) The 2nd memset() argument '1.0f+i' is a float, its representation is implementation defined.\n", errout.str());
    }
};

REGISTER_TEST(TestFunctions)
