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
#include "checkfunctions.h"
#include "testsuite.h"
#include <tinyxml2.h>


class TestFunctions : public TestFixture {
public:
    TestFunctions() : TestFixture("TestFunctions") {
    }

private:
    Settings settings;

    void run() {
        settings.addEnabled("style");
        settings.addEnabled("warning");
        settings.addEnabled("portability");
        settings.standards.posix = true;
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

        // Check...
        checkFunctions.checkIgnoredReturnValue();

        // Simplify...
        tokenizer.simplifyTokenList2();

        // Check...
        checkFunctions.checkProhibitedFunctions();
        checkFunctions.checkMathFunctions();
        checkFunctions.invalidFunctionUsage();
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
              "    printf(\"Magic guess: %d\n\", getpwent());\n"
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

        check("int f() { strtol(a,b,sizeof(a)!=12); }");
        ASSERT_EQUALS("[test.cpp:1]: (error) Invalid strtol() argument nr 3. The value is 0 or 1 (comparison result) but the valid values are '0,2:36'.\n", errout.str());

        check("int f() { strtol(a,b,1); }");
        ASSERT_EQUALS("[test.cpp:1]: (error) Invalid strtol() argument nr 3. The value is 1 but the valid values are '0,2:36'.\n", errout.str());

        check("int f() { strtol(a,b,10); }");
        ASSERT_EQUALS("", errout.str());
    }

    void mathfunctionCall_sqrt() {
        // sqrt, sqrtf, sqrtl
        check("void foo()\n"
              "{\n"
              "    std::cout <<  sqrt(-1) << std::endl;\n"
              "    std::cout <<  sqrtf(-1) << std::endl;\n"
              "    std::cout <<  sqrtl(-1) << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Passing value -1 to sqrt() leads to implementation-defined result.\n"
                      "[test.cpp:4]: (warning) Passing value -1 to sqrtf() leads to implementation-defined result.\n"
                      "[test.cpp:5]: (warning) Passing value -1 to sqrtl() leads to implementation-defined result.\n", errout.str());

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
        // log,log10,logf,logl,log10f,log10l
        check("void foo()\n"
              "{\n"
              "    std::cout <<  log(-2) << std::endl;\n"
              "    std::cout <<  logf(-2) << std::endl;\n"
              "    std::cout <<  logl(-2) << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Passing value -2 to log() leads to implementation-defined result.\n"
                      "[test.cpp:4]: (warning) Passing value -2 to logf() leads to implementation-defined result.\n"
                      "[test.cpp:5]: (warning) Passing value -2 to logl() leads to implementation-defined result.\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::cout <<  log(-1) << std::endl;\n"
              "    std::cout <<  logf(-1) << std::endl;\n"
              "    std::cout <<  logl(-1) << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Passing value -1 to log() leads to implementation-defined result.\n"
                      "[test.cpp:4]: (warning) Passing value -1 to logf() leads to implementation-defined result.\n"
                      "[test.cpp:5]: (warning) Passing value -1 to logl() leads to implementation-defined result.\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::cout <<  log(-1.0) << std::endl;\n"
              "    std::cout <<  logf(-1.0) << std::endl;\n"
              "    std::cout <<  logl(-1.0) << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Passing value -1.0 to log() leads to implementation-defined result.\n"
                      "[test.cpp:4]: (warning) Passing value -1.0 to logf() leads to implementation-defined result.\n"
                      "[test.cpp:5]: (warning) Passing value -1.0 to logl() leads to implementation-defined result.\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::cout <<  log(-0.1) << std::endl;\n"
              "    std::cout <<  logf(-0.1) << std::endl;\n"
              "    std::cout <<  logl(-0.1) << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Passing value -0.1 to log() leads to implementation-defined result.\n"
                      "[test.cpp:4]: (warning) Passing value -0.1 to logf() leads to implementation-defined result.\n"
                      "[test.cpp:5]: (warning) Passing value -0.1 to logl() leads to implementation-defined result.\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::cout <<  log(0) << std::endl;\n"
              "    std::cout <<  logf(0.) << std::endl;\n"
              "    std::cout <<  logl(0.0) << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Passing value 0 to log() leads to implementation-defined result.\n"
                      "[test.cpp:4]: (warning) Passing value 0. to logf() leads to implementation-defined result.\n"
                      "[test.cpp:5]: (warning) Passing value 0.0 to logl() leads to implementation-defined result.\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::cout <<  log(1E-3)    << std::endl;\n"
              "    std::cout <<  logf(1E-3)   << std::endl;\n"
              "    std::cout <<  logl(1E-3)   << std::endl;\n"
              "    std::cout <<  log(1.0E-3)  << std::endl;\n"
              "    std::cout <<  logf(1.0E-3) << std::endl;\n"
              "    std::cout <<  logl(1.0E-3) << std::endl;\n"
              "    std::cout <<  log(1.0E+3)  << std::endl;\n"
              "    std::cout <<  logf(1.0E+3) << std::endl;\n"
              "    std::cout <<  logl(1.0E+3) << std::endl;\n"
              "    std::cout <<  log(2.0)     << std::endl;\n"
              "    std::cout <<  logf(2.0)    << std::endl;\n"
              "    std::cout <<  logf(2.0f)   << std::endl;\n"
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
        ASSERT_EQUALS("[test.cpp:3]: (warning) Passing value 1.1 to acos() leads to implementation-defined result.\n"
                      "[test.cpp:4]: (warning) Passing value 1.1 to acosf() leads to implementation-defined result.\n"
                      "[test.cpp:5]: (warning) Passing value 1.1 to acosl() leads to implementation-defined result.\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::cout <<  acos(-1.1) << std::endl;\n"
              "    std::cout <<  acosf(-1.1) << std::endl;\n"
              "    std::cout <<  acosl(-1.1) << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Passing value -1.1 to acos() leads to implementation-defined result.\n"
                      "[test.cpp:4]: (warning) Passing value -1.1 to acosf() leads to implementation-defined result.\n"
                      "[test.cpp:5]: (warning) Passing value -1.1 to acosl() leads to implementation-defined result.\n", errout.str());
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
        ASSERT_EQUALS("[test.cpp:3]: (warning) Passing value 1.1 to asin() leads to implementation-defined result.\n"
                      "[test.cpp:4]: (warning) Passing value 1.1 to asinf() leads to implementation-defined result.\n"
                      "[test.cpp:5]: (warning) Passing value 1.1 to asinl() leads to implementation-defined result.\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    std::cout <<  asin(-1.1) << std::endl;\n"
              "    std::cout <<  asinf(-1.1) << std::endl;\n"
              "    std::cout <<  asinl(-1.1) << std::endl;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Passing value -1.1 to asin() leads to implementation-defined result.\n"
                      "[test.cpp:4]: (warning) Passing value -1.1 to asinf() leads to implementation-defined result.\n"
                      "[test.cpp:5]: (warning) Passing value -1.1 to asinl() leads to implementation-defined result.\n", errout.str());
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
    }
};

REGISTER_TEST(TestFunctions)
