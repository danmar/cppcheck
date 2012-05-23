/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2012 Daniel Marjamäki and Cppcheck team.
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

#include "checkio.h"
#include "testsuite.h"
#include "tokenize.h"
#include <sstream>

extern std::ostringstream errout;

class TestIO : public TestFixture {
public:
    TestIO() : TestFixture("TestIO")
    { }

private:

    void run() {
        TEST_CASE(coutCerrMisusage);

        TEST_CASE(wrongMode_simple);
        TEST_CASE(wrongMode_complex);
        TEST_CASE(useClosedFile);
        TEST_CASE(fileIOwithoutPositioning);
        TEST_CASE(fflushOnInputStream);

        TEST_CASE(testScanf1); // Scanf without field limiters
        TEST_CASE(testScanf2);
        TEST_CASE(testScanf3);
        TEST_CASE(testScanf4);

        TEST_CASE(testScanfArgument);
        TEST_CASE(testPrintfArgument);
    }

    void check(const char code[]) {
        // Clear the error buffer..
        errout.str("");

        Settings settings;
        settings.addEnabled("style");

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        // Check..
        CheckIO checkIO(&tokenizer, &settings, this);
        checkIO.checkWrongPrintfScanfArguments();

        // Simplify token list..
        tokenizer.simplifyTokenList();
        checkIO.checkCoutCerrMisusage();
        checkIO.checkFileUsage();
        checkIO.invalidScanf();
    }




    void coutCerrMisusage() {
        check(
            "void foo() {\n"
            "  std::cout << std::cout;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:2]: (error) Invalid usage of output stream: '<< std::cout'.\n", errout.str());

        check(
            "void foo() {\n"
            "  std::cout << \"xyz\" << std::cout;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:2]: (error) Invalid usage of output stream: '<< std::cout'.\n", errout.str());

        check(
            "void foo(int i) {\n"
            "  std::cout << i << std::cerr;\n"
            "}");
        ASSERT_EQUALS("[test.cpp:2]: (error) Invalid usage of output stream: '<< std::cerr'.\n", errout.str());

        check(
            "void foo() {\n"
            "  std::cout << \"xyz\";\n"
            "  std::cout << \"xyz\";\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check(
            "void foo() {\n"
            "  std::cout << std::cout.good();\n"
            "}");
        ASSERT_EQUALS("", errout.str());

        check(
            "void foo() {\n"
            "  MACRO(std::cout <<, << std::cout)\n"
            "}");
        ASSERT_EQUALS("", errout.str());
    }



    void wrongMode_simple() {
        // Read mode
        check("void foo(FILE*& f) {\n"
              "    f = fopen(name, \"r\");\n"
              "    fread(buffer, 5, 6, f);\n"
              "    rewind(f);\n"
              "    fwrite(buffer, 5, 6, f);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Write operation on a file that was only opened for reading.\n", errout.str());

        check("void foo(FILE*& f) {\n"
              "    f = fopen(name, \"r+\");\n"
              "    fwrite(buffer, 5, 6, f);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // Write mode
        check("void foo(FILE*& f) {\n"
              "    f = fopen(name, \"w\");\n"
              "    fwrite(buffer, 5, 6, f);\n"
              "    rewind(f);\n"
              "    fread(buffer, 5, 6, f);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Read operation on a file that was only opened for writing.\n", errout.str());

        check("void foo(FILE*& f) {\n"
              "    f = fopen(name, \"w+\");\n"
              "    fread(buffer, 5, 6, f);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // Append mode
        check("void foo(FILE*& f) {\n"
              "    f = fopen(name, \"a\");\n"
              "    fwrite(buffer, 5, 6, f);\n"
              "    rewind(f);\n"
              "    fread(buffer, 5, 6, f);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Read operation on a file that was only opened for writing.\n", errout.str());

        check("void foo(FILE*& f) {\n"
              "    f = fopen(name, \"a+\");\n"
              "    fread(buffer, 5, 6, f);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // Variable declared locally
        check("void foo() {\n"
              "    FILE* f = fopen(name, \"r\");\n"
              "    fwrite(buffer, 5, 6, f);\n"
              "    fclose(f);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Write operation on a file that was only opened for reading.\n", errout.str());

        // Call unknown function
        check("void foo(FILE*& f) {\n"
              "    f = fopen(name, \"a\");\n"
              "    fwrite(buffer, 5, 6, f);\n"
              "    bar(f);\n"
              "    fread(buffer, 5, 6, f);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo(FILE*& f) {\n"
              "    f = fopen(name, \"a\");\n"
              "    fwrite(buffer, 5, 6, f);\n"
              "    clearerr(f);\n"
              "    fread(buffer, 5, 6, f);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Read operation on a file that was only opened for writing.\n", errout.str());

        // freopen and tmpfile
        check("void foo(FILE*& f) {\n"
              "    f = freopen(name, \"r\", f);\n"
              "    fwrite(buffer, 5, 6, f);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Write operation on a file that was only opened for reading.\n", errout.str());

        // Crash tests
        check("void foo(FILE*& f) {\n"
              "    f = fopen(name, mode);\n" // No assertion failure (#3830)
              "    fwrite(buffer, 5, 6, f);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void fopen(std::string const &filepath, std::string const &mode);"); // #3832
    }

    void wrongMode_complex() {
        check("void foo(FILE* f) {\n"
              "    if(a) f = fopen(name, \"w\");\n"
              "    else  f = fopen(name, \"r\");\n"
              "    if(a) fwrite(buffer, 5, 6, f);\n"
              "    else  fread(buffer, 5, 6, f);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo() {\n"
              "    FILE* f;\n"
              "    if(a) f = fopen(name, \"w\");\n"
              "    else  f = fopen(name, \"r\");\n"
              "    if(a) fwrite(buffer, 5, 6, f);\n"
              "    else  fread(buffer, 5, 6, f);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo() {\n"
              "    FILE* f = fopen(name, \"w\");\n"
              "    if(a) fwrite(buffer, 5, 6, f);\n"
              "    else  fread(buffer, 5, 6, f);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Read operation on a file that was only opened for writing.\n", errout.str());
    }

    void useClosedFile() {
        check("void foo(FILE*& f) {\n"
              "    fclose(f);\n"
              "    fwrite(buffer, 5, 6, f);\n"
              "    clearerr(f);\n"
              "    fread(buffer, 5, 6, f);\n"
              "    ungetc('a', f);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Used file that is not opened.\n"
                      "[test.cpp:4]: (error) Used file that is not opened.\n"
                      "[test.cpp:5]: (error) Used file that is not opened.\n"
                      "[test.cpp:6]: (error) Used file that is not opened.\n", errout.str());

        check("void foo(FILE*& f) {\n"
              "    if(!ferror(f)) {\n"
              "        fclose(f);\n"
              "        return;"
              "    }\n"
              "    fwrite(buffer, 5, 6, f);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo(FILE*& f) {\n"
              "    fclose(f);\n"
              "    f = fopen(name, \"r\");\n"
              "    fread(buffer, 5, 6, f);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo(FILE*& f) {\n"
              "    f = fopen(name, \"r\");\n"
              "    f = g;\n"
              "    fwrite(buffer, 5, 6, f);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void fileIOwithoutPositioning() {
        check("void foo(FILE* f) {\n"
              "    fwrite(buffer, 5, 6, f);\n"
              "    fread(buffer, 5, 6, f);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Read and write operations without a call to a positioning function (fseek, fsetpos or rewind) or fflush inbetween result in undefined behaviour.\n", errout.str());

        check("void foo(FILE* f) {\n"
              "    fread(buffer, 5, 6, f);\n"
              "    fwrite(buffer, 5, 6, f);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Read and write operations without a call to a positioning function (fseek, fsetpos or rewind) or fflush inbetween result in undefined behaviour.\n", errout.str());

        check("void foo(FILE* f, bool read) {\n"
              "    if(read)\n"
              "        fread(buffer, 5, 6, f);\n"
              "    else\n"
              "        fwrite(buffer, 5, 6, f);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo(FILE* f) {\n"
              "    fread(buffer, 5, 6, f);\n"
              "    fflush(f);\n"
              "    fwrite(buffer, 5, 6, f);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo(FILE* f) {\n"
              "    fread(buffer, 5, 6, f);\n"
              "    rewind(f);\n"
              "    fwrite(buffer, 5, 6, f);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo(FILE* f) {\n"
              "    fread(buffer, 5, 6, f);\n"
              "    fsetpos(f, pos);\n"
              "    fwrite(buffer, 5, 6, f);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo(FILE* f) {\n"
              "    fread(buffer, 5, 6, f);\n"
              "    fseek(f, 0, SEEK_SET);\n"
              "    fwrite(buffer, 5, 6, f);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo(FILE* f) {\n"
              "    fread(buffer, 5, 6, f);\n"
              "    long pos = ftell(f);\n"
              "    fwrite(buffer, 5, 6, f);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Read and write operations without a call to a positioning function (fseek, fsetpos or rewind) or fflush inbetween result in undefined behaviour.\n", errout.str());
    }

    void fflushOnInputStream() {
        check("void foo()\n"
              "{\n"
              "    fflush(stdin);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) fflush() called on input stream \"stdin\" may result in undefined behaviour.\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    fflush(stdout);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }




    void testScanf1() {
        check("#include <stdio.h>\n"
              "int main(int argc, char **argv)\n"
              "{\n"
              "    int a, b;\n"
              "    FILE *file = fopen(\"test\", \"r\");\n"
              "    b = fscanf(file, \"aa %ds\", &a);\n"
              "    c = scanf(\"aa %ds\", &a);\n"
              "    b = fscanf(file, \"aa%%ds\", &a);\n"
              "    fclose(file);\n"
              "    return b;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8]: (warning) fscanf format string has 0 parameters but 1 are given\n"
                      "[test.cpp:6]: (warning) scanf without field width limits can crash with huge input data\n"
                      "[test.cpp:7]: (warning) scanf without field width limits can crash with huge input data\n", errout.str());
    }

    void testScanf2() {
        check("#include <stdio.h>\n"
              "int main(int argc, char **argv)\n"
              "{\n"
              "    int a, b;\n"
              "    FILE *file = fopen(\"test\", \"r\");\n"
              "    b = fscanf(file, \"aa%%%ds\", &a);\n"
              "    c = scanf(\"aa %%%ds\", &a);\n"
              "    b = fscanf(file, \"aa%%ds\", &a);\n"
              "    fclose(file);\n"
              "    return b;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:8]: (warning) fscanf format string has 0 parameters but 1 are given\n"
                      "[test.cpp:6]: (warning) scanf without field width limits can crash with huge input data\n"
                      "[test.cpp:7]: (warning) scanf without field width limits can crash with huge input data\n", errout.str());
    }

    void testScanf3() {
        check("#include <stdio.h>\n"
              "int main(int argc, char **argv)\n"
              "{\n"
              "    char a[32];\n"
              "    int b, c;\n"
              "    FILE *file = fopen(\"test\", \"r\");\n"
              "    c = fscanf(file, \"%[^ ] %d\n\", a, &b);\n"
              "    fclose(file);\n"
              "    return c;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("#include <stdio.h>\n"
              "int main(int argc, char **argv)\n"
              "{\n"
              "    char a[32];\n"
              "    int b;\n"
              "    FILE *file = fopen(\"test\", \"r\");\n"
              "    b = fscanf(file, \"%[^ \n\", a);\n"
              "    fclose(file);\n"
              "    return b;\n"
              "}");
        ASSERT_EQUALS("[test.cpp:7]: (warning) fscanf format string has 0 parameters but 1 are given\n", errout.str());
    }

    void testScanf4() {
        check("void f() {\n"
              "    char c;\n"
              "    scanf(\"%c\", &c);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }




    void testScanfArgument() {
        check("void foo() {\n"
              "    scanf(\"%1d\", &foo);\n"
              "    sscanf(bar, \"%1d\", &foo);\n"
              "    scanf(\"%1u%1u\", &foo, bar());\n"
              "    scanf(\"%*1x %1x %29s\", &count, KeyName);\n" // #3373
              "    fscanf(f, \"%7ms\", &ref);\n" // #3461
              "    sscanf(ip_port, \"%*[^:]:%d\", &port);\n" // #3468
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo() {\n"
              "    scanf(\"\", &foo);\n"
              "    scanf(\"%1d\", &foo, &bar);\n"
              "    fscanf(bar, \"%1d\", &foo, &bar);\n"
              "    scanf(\"%*1x %1x %29s\", &count, KeyName, foo);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) scanf format string has 0 parameters but 1 are given\n"
                      "[test.cpp:3]: (warning) scanf format string has 1 parameters but 2 are given\n"
                      "[test.cpp:4]: (warning) fscanf format string has 1 parameters but 2 are given\n"
                      "[test.cpp:5]: (warning) scanf format string has 2 parameters but 3 are given\n", errout.str());

        check("void foo() {\n"
              "    scanf(\"%1d\");\n"
              "    scanf(\"%1u%1u\", bar());\n"
              "    sscanf(bar, \"%1d%1d\", &foo);\n"
              "    scanf(\"%*1x %1x %29s\", &count);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (error) scanf format string has 1 parameters but only 0 are given\n"
                      "[test.cpp:3]: (error) scanf format string has 2 parameters but only 1 are given\n"
                      "[test.cpp:4]: (error) sscanf format string has 2 parameters but only 1 are given\n"
                      "[test.cpp:5]: (error) scanf format string has 2 parameters but only 1 are given\n", errout.str());
    }

    void testPrintfArgument() {
        check("void foo() {\n"
              "    printf(\"%u\");\n"
              "    printf(\"%u%s\", 123);\n"
              "    printf(\"%u%s%d\", 0, bar());\n"
              "    printf(\"%u%%%s%d\", 0, bar());\n"
              "    printf(\"%udfd%%dfa%s%d\", 0, bar());\n"
              "    fprintf(stderr,\"%u%s\");\n"
              "    snprintf(str,10,\"%u%s\");\n"
              "    sprintf(string1, \"%-*.*s\", 32, string2);\n" // #3364
              "    snprintf(a, 9, \"%s%d\", \"11223344\");\n" // #3655
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (error) printf format string has 1 parameters but only 0 are given\n"
                      "[test.cpp:3]: (error) printf format string has 2 parameters but only 1 are given\n"
                      "[test.cpp:4]: (error) printf format string has 3 parameters but only 2 are given\n"
                      "[test.cpp:5]: (error) printf format string has 3 parameters but only 2 are given\n"
                      "[test.cpp:6]: (error) printf format string has 3 parameters but only 2 are given\n"
                      "[test.cpp:7]: (error) fprintf format string has 2 parameters but only 0 are given\n"
                      "[test.cpp:8]: (error) snprintf format string has 2 parameters but only 0 are given\n"
                      "[test.cpp:9]: (error) sprintf format string has 3 parameters but only 2 are given\n"
                      "[test.cpp:10]: (error) snprintf format string has 2 parameters but only 1 are given\n", errout.str());

        check("void foo(char *str) {\n"
              "    printf(\"\", 0);\n"
              "    printf(\"%u\", 123, bar());\n"
              "    printf(\"%u%s\", 0, bar(), 43123);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) printf format string has 0 parameters but 1 are given\n"
                      "[test.cpp:3]: (warning) printf format string has 1 parameters but 2 are given\n"
                      "[test.cpp:4]: (warning) printf format string has 2 parameters but 3 are given\n", errout.str());

        check("void foo(char *str) {\n"
              "    printf(\"%u\", 0);\n"
              "    printf(\"%u%s\", 123, bar());\n"
              "    printf(\"%u%s%d\", 0, bar(), 43123);\n"
              "    printf(\"%u%%%s%d\", 0, bar(), 43123);\n"
              "    printf(\"%udfd%%dfa%s%d\", 0, bar(), 43123);\n"
              "    printf(\"%\"PRId64\"\n\", 123);\n"
              "    fprintf(stderr,\"%\"PRId64\"\n\", 123);\n"
              "    snprintf(str,10,\"%\"PRId64\"\n\", 123);\n"
              "    fprintf(stderr, \"error: %m\n\");\n" // #3339
              "    printf(\"string: %.*s\n\", len, string);\n" // #3311
              "    fprintf(stderr, \"%*cText.\n\", indent, ' ');\n" // #3313
              "    sprintf(string1, \"%*\", 32);\n" // #3364
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo(char* s, const char* s2, std::string s3, int i) {\n"
              "    printf(\"%s%s\", s, s2);\n"
              "    printf(\"%s\", i);\n"
              "    printf(\"%i%s\", i, i);\n"
              "    printf(\"%s\", s3);\n"
              "    printf(\"%s\", \"s4\");\n"
              "    printf(\"%u\", s);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) %s in format string (no. 1) requires a char* given in the argument list\n"
                      "[test.cpp:4]: (warning) %s in format string (no. 2) requires a char* given in the argument list\n"
                      "[test.cpp:5]: (warning) %s in format string (no. 1) requires a char* given in the argument list\n", errout.str());

        check("void foo(const int* cpi, const int ci, int i, int* pi, std::string s) {\n"
              "    printf(\"%n\", cpi);\n"
              "    printf(\"%n\", ci);\n"
              "    printf(\"%n\", i);\n"
              "    printf(\"%n\", pi);\n"
              "    printf(\"%n\", s);\n"
              "    printf(\"%n\", \"s4\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) %n in format string (no. 1) requires a pointer to an non-const integer given in the argument list\n"
                      "[test.cpp:3]: (warning) %n in format string (no. 1) requires a pointer to an non-const integer given in the argument list\n"
                      "[test.cpp:4]: (warning) %n in format string (no. 1) requires a pointer to an non-const integer given in the argument list\n"
                      "[test.cpp:6]: (warning) %n in format string (no. 1) requires a pointer to an non-const integer given in the argument list\n"
                      "[test.cpp:7]: (warning) %n in format string (no. 1) requires a pointer to an non-const integer given in the argument list\n", errout.str());

        check("class foo {};\n"
              "void foo(const int* cpi, foo f, bar b, bar* bp, double d) {\n"
              "    printf(\"%i\", f);\n"
              "    printf(\"%c\", \"s4\");\n"
              "    printf(\"%o\", d);\n"
              "    printf(\"%i\", cpi);\n"
              "    printf(\"%u\", b);\n"
              "    printf(\"%u\", bp);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) %i in format string (no. 1) requires an integer given in the argument list\n"
                      "[test.cpp:4]: (warning) %c in format string (no. 1) requires an integer given in the argument list\n"
                      "[test.cpp:5]: (warning) %o in format string (no. 1) requires an integer given in the argument list\n", errout.str());

        check("class foo {};\n"
              "void foo(const int* cpi, foo f, bar b, bar* bp, char c) {\n"
              "    printf(\"%p\", f);\n"
              "    printf(\"%p\", c);\n"
              "    printf(\"%p\", bp);\n"
              "    printf(\"%p\", cpi);\n"
              "    printf(\"%p\", b);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) %p in format string (no. 1) requires an integer or pointer given in the argument list\n"
                      "[test.cpp:4]: (warning) %p in format string (no. 1) requires an integer or pointer given in the argument list\n", errout.str());

        check("class foo {};\n"
              "void foo(const int* cpi, foo f, bar b, bar* bp, double d) {\n"
              "    printf(\"%e\", f);\n"
              "    printf(\"%E\", \"s4\");\n"
              "    printf(\"%f\", cpi);\n"
              "    printf(\"%G\", bp);\n"
              "    printf(\"%f\", d);\n"
              "    printf(\"%f\", b);\n"
              "    printf(\"%f\", (float)cpi);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) %e in format string (no. 1) requires a floating point number given in the argument list\n"
                      "[test.cpp:4]: (warning) %E in format string (no. 1) requires a floating point number given in the argument list\n"
                      "[test.cpp:5]: (warning) %f in format string (no. 1) requires a floating point number given in the argument list\n"
                      "[test.cpp:6]: (warning) %G in format string (no. 1) requires a floating point number given in the argument list\n", errout.str());

        check("class foo;\n"
              "void foo(foo f) {\n"
              "    printf(\"%u\", f);\n"
              "    printf(\"%f\", f);\n"
              "    printf(\"%p\", f);\n"
              "}");
        TODO_ASSERT_EQUALS("[test.cpp:3]: (warning) %u in format string (no. 1) requires an integer given in the argument list\n"
                           "[test.cpp:4]: (warning) %f in format string (no. 1) requires an integer given in the argument list\n"
                           "[test.cpp:5]: (warning) %p in format string (no. 1) requires an integer given in the argument list\n", "", errout.str());
    }
};

REGISTER_TEST(TestIO)
