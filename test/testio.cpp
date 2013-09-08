/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2013 Daniel Marjamäki and Cppcheck team.
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
    TestIO() : TestFixture("TestIO") {
    }

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
        TEST_CASE(testScanf4); // #ticket 2553

        TEST_CASE(testScanfArgument);
        TEST_CASE(testPrintfArgument);
        TEST_CASE(testPosixPrintfScanfParameterPosition);  // #4900

        TEST_CASE(testMicrosoftPrintfArgument); // ticket #4902

        TEST_CASE(testlibrarycfg); // library configuration
    }

    void check(const char code[], bool inconclusive = false, bool portability = false, Settings::PlatformType platform = Settings::Unspecified, Library *lib = NULL) {
        // Clear the error buffer..
        errout.str("");

        Settings settings;
        settings.addEnabled("warning");
        settings.addEnabled("style");
        if (portability)
            settings.addEnabled("portability");
        settings.inconclusive = inconclusive;
        settings.platform(platform);

        if (lib)
            settings.library = *lib;

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
        ASSERT_EQUALS("[test.cpp:5]: (error) Write operation on a file that was opened only for reading.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:5]: (error) Read operation on a file that was opened only for writing.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:5]: (error) Read operation on a file that was opened only for writing.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:3]: (error) Write operation on a file that was opened only for reading.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:5]: (error) Read operation on a file that was opened only for writing.\n", errout.str());

        // freopen and tmpfile
        check("void foo(FILE*& f) {\n"
              "    f = freopen(name, \"r\", f);\n"
              "    fwrite(buffer, 5, 6, f);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Write operation on a file that was opened only for reading.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:4]: (error) Read operation on a file that was opened only for writing.\n", errout.str());
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

        check("void foo() {\n"
              "    FILE* f;\n"
              "    fwrite(buffer, 5, 6, f);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Used file that is not opened.\n", errout.str());

        check("void foo() {\n"
              "    FILE* f(stdout);\n"
              "    fwrite(buffer, 5, 6, f);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo() {\n" // #3965
              "    FILE* f[3];\n"
              "    f[0] = fopen(name, mode);\n"
              "    fclose(f[0]);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #4368: multiple functions
        check("static FILE *fp = NULL;\n"
              "\n"
              "void close()\n"
              "{\n"
              "  fclose(fp);\n"
              "}\n"
              "\n"
              "void dump()\n"
              "{\n"
              "  if (fp == NULL) return;\n"
              "  fprintf(fp, \"Here's the output.\\n\");\n"
              "}\n"
              "\n"
              "int main()\n"
              "{\n"
              "  fp = fopen(\"test.txt\", \"w\");\n"
              "  dump();\n"
              "  close();\n"
              "  return 0;\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("static FILE *fp = NULL;\n"
              "\n"
              "void close()\n"
              "{\n"
              "  fclose(fp);\n"
              "}\n"
              "\n"
              "void dump()\n"
              "{\n"
              "  fclose(fp);\n"
              "  fprintf(fp, \"Here's the output.\\n\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:11]: (error) Used file that is not opened.\n", errout.str());

        // #4466
        check("void chdcd_parse_nero(FILE *infile) {\n"
              "    switch (mode) {\n"
              "        case 0x0300:\n"
              "            fclose(infile);\n"
              "            return;\n"
              "        case 0x0500:\n"
              "            fclose(infile);\n"
              "            return;\n"
              "    }\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // #4649
        check("void foo() {\n"
              "    struct {FILE *f1; FILE *f2;} a;\n"
              "    a.f1 = fopen(name,mode);\n"
              "    a.f2 = fopen(name,mode);\n"
              "    fclose(a.f1);\n"
              "    fclose(a.f2);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }

    void fileIOwithoutPositioning() {
        check("void foo(FILE* f) {\n"
              "    fwrite(buffer, 5, 6, f);\n"
              "    fread(buffer, 5, 6, f);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Read and write operations without a call to a positioning function (fseek, fsetpos or rewind) or fflush in between result in undefined behaviour.\n", errout.str());

        check("void foo(FILE* f) {\n"
              "    fread(buffer, 5, 6, f);\n"
              "    fwrite(buffer, 5, 6, f);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Read and write operations without a call to a positioning function (fseek, fsetpos or rewind) or fflush in between result in undefined behaviour.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:4]: (error) Read and write operations without a call to a positioning function (fseek, fsetpos or rewind) or fflush in between result in undefined behaviour.\n", errout.str());
    }

    void fflushOnInputStream() {
        check("void foo()\n"
              "{\n"
              "    fflush(stdin);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) fflush() called on input stream 'stdin' results in undefined behaviour.\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    fflush(stdout);\n"
              "}");
        ASSERT_EQUALS("", errout.str());
    }




    void testScanf1() {
        check("void foo() {\n"
              "    int a, b;\n"
              "    FILE *file = fopen(\"test\", \"r\");\n"
              "    a = fscanf(file, \"aa %s\", bar);\n"
              "    b = scanf(\"aa %S\", bar);\n"
              "    b = scanf(\"aa %ls\", bar);\n"
              "    sscanf(foo, \"%[^~]\", bar);\n"
              "    scanf(\"%dx%s\", &b, bar);\n"
              "    fclose(file);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (warning) scanf without field width limits can crash with huge input data.\n"
                      "[test.cpp:5]: (warning) scanf without field width limits can crash with huge input data.\n"
                      "[test.cpp:6]: (warning) scanf without field width limits can crash with huge input data.\n"
                      "[test.cpp:7]: (warning) scanf without field width limits can crash with huge input data.\n"
                      "[test.cpp:8]: (warning) scanf without field width limits can crash with huge input data.\n", errout.str());
    }

    void testScanf2() {
        check("void foo() {\n"
              "    scanf(\"%5s\", bar);\n" // Width specifier given
              "    scanf(\"%5[^~]\", bar);\n" // Width specifier given
              "    scanf(\"aa%%s\", bar);\n" // No %s
              "    scanf(\"aa%d\", &a);\n" // No %s
              "    scanf(\"aa%ld\", &a);\n" // No %s
              "    scanf(\"%*[^~]\");\n" // Ignore input
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (warning) scanf format string has 0 parameters but 1 are given.\n", errout.str());
    }

    void testScanf3() {
        check("void foo() {\n"
              "    scanf(\"%d\", &a);\n"
              "    scanf(\"%n\", &a);\n" // No warning on %n, since it doesn't expect user input
              "    scanf(\"%c\", &c);\n" // No warning on %c; it expects only one character
              "}", false, true, Settings::Unspecified);
        ASSERT_EQUALS("[test.cpp:2]: (portability) scanf without field width limits can crash with huge input data on some versions of libc.\n", errout.str());

        check("void foo() {\n"
              "    scanf(\"%d\", &a);\n"
              "}", false, true, Settings::Win32A);
        ASSERT_EQUALS("", errout.str());
    }

    void testScanf4() { // ticket #2553

        check("void f()\n"
              "{\n"
              "  char str [8];\n"
              "  scanf (\"%70s\",str);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Width 70 given in format string (no. 1) is larger than destination buffer 'str[8]', use %7s to prevent overflowing it.\n", errout.str());
    }




    void testScanfArgument() {
        check("void foo() {\n"
              "    scanf(\"%1d\", &foo);\n"
              "    sscanf(bar, \"%1d\", &foo);\n"
              "    scanf(\"%1u%1u\", &foo, bar());\n"
              "    scanf(\"%*1x %1x %29s\", &count, KeyName);\n" // #3373
              "    fscanf(f, \"%7ms\", &ref);\n" // #3461
              "    sscanf(ip_port, \"%*[^:]:%4d\", &port);\n" // #3468
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo() {\n"
              "    scanf(\"\", &foo);\n"
              "    scanf(\"%1d\", &foo, &bar);\n"
              "    fscanf(bar, \"%1d\", &foo, &bar);\n"
              "    scanf(\"%*1x %1x %29s\", &count, KeyName, foo);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) scanf format string has 0 parameters but 1 are given.\n"
                      "[test.cpp:3]: (warning) scanf format string has 1 parameters but 2 are given.\n"
                      "[test.cpp:4]: (warning) fscanf format string has 1 parameters but 2 are given.\n"
                      "[test.cpp:5]: (warning) scanf format string has 2 parameters but 3 are given.\n", errout.str());

        check("void foo() {\n"
              "    scanf(\"%1d\");\n"
              "    scanf(\"%1u%1u\", bar());\n"
              "    sscanf(bar, \"%1d%1d\", &foo);\n"
              "    scanf(\"%*1x %1x %29s\", &count);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (error) scanf format string has 1 parameters but only 0 are given.\n"
                      "[test.cpp:3]: (error) scanf format string has 2 parameters but only 1 are given.\n"
                      "[test.cpp:4]: (error) sscanf format string has 2 parameters but only 1 are given.\n"
                      "[test.cpp:5]: (error) scanf format string has 2 parameters but only 1 are given.\n", errout.str());

        check("void foo() {\n"
              "    char input[10];\n"
              "    char output[5];\n"
              "    sscanf(input, \"%3s\", output);\n"
              "    sscanf(input, \"%4s\", output);\n"
              "    sscanf(input, \"%5s\", output);\n"
              "}", false);
        ASSERT_EQUALS("[test.cpp:6]: (error) Width 5 given in format string (no. 1) is larger than destination buffer 'output[5]', use %4s to prevent overflowing it.\n", errout.str());

        check("void foo() {\n"
              "    char input[10];\n"
              "    char output[5];\n"
              "    sscanf(input, \"%s\", output);\n"
              "    sscanf(input, \"%3s\", output);\n"
              "    sscanf(input, \"%4s\", output);\n"
              "    sscanf(input, \"%5s\", output);\n"
              "}", true);
        ASSERT_EQUALS("[test.cpp:5]: (warning, inconclusive) Width 3 given in format string (no. 1) is smaller than destination buffer 'output[5]'.\n"
                      "[test.cpp:7]: (error) Width 5 given in format string (no. 1) is larger than destination buffer 'output[5]', use %4s to prevent overflowing it.\n"
                      "[test.cpp:4]: (warning) scanf without field width limits can crash with huge input data.\n", errout.str());

        check("void foo() {\n"
              "    const size_t BUFLENGTH(2048);\n"
              "    typedef char bufT[BUFLENGTH];\n"
              "    bufT line= {0};\n"
              "    bufT projectId= {0};\n"
              "    const int scanrc=sscanf(line, \"Project(\\\"{%36s}\\\")\", projectId);\n"
              "    sscanf(input, \"%5s\", output);\n"
              "}", true);
        ASSERT_EQUALS("", errout.str());
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
        ASSERT_EQUALS("[test.cpp:2]: (error) printf format string has 1 parameters but only 0 are given.\n"
                      "[test.cpp:3]: (error) printf format string has 2 parameters but only 1 are given.\n"
                      "[test.cpp:4]: (error) printf format string has 3 parameters but only 2 are given.\n"
                      "[test.cpp:5]: (error) printf format string has 3 parameters but only 2 are given.\n"
                      "[test.cpp:6]: (error) printf format string has 3 parameters but only 2 are given.\n"
                      "[test.cpp:7]: (error) fprintf format string has 2 parameters but only 0 are given.\n"
                      "[test.cpp:8]: (error) snprintf format string has 2 parameters but only 0 are given.\n"
                      "[test.cpp:9]: (error) sprintf format string has 3 parameters but only 2 are given.\n"
                      "[test.cpp:10]: (error) snprintf format string has 2 parameters but only 1 are given.\n", errout.str());

        check("void foo(char *str) {\n"
              "    printf(\"\", 0);\n"
              "    printf(\"%u\", 123, bar());\n"
              "    printf(\"%u%s\", 0, bar(), 43123);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) printf format string has 0 parameters but 1 are given.\n"
                      "[test.cpp:3]: (warning) printf format string has 1 parameters but 2 are given.\n"
                      "[test.cpp:4]: (warning) printf format string has 2 parameters but 3 are given.\n", errout.str());

        check("void foo() {\n" // swprintf exists as MSVC extension and as standard function: #4790
              "    swprintf(string1, L\"%u\", 32, string2);\n" // MSVC implementation
              "    swprintf(string1, L\"%s%s\", L\"a\", string2);\n" // MSVC implementation
              "    swprintf(string1, 6, L\"%u\", 32, string2);\n" // Standard implementation
              "    swprintf(string1, 6, L\"%u%s\", 32, string2);\n" // Standard implementation
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) swprintf format string has 1 parameters but 2 are given.\n"
                      "[test.cpp:4]: (warning) swprintf format string has 1 parameters but 2 are given.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:3]: (warning) %s in format string (no. 1) requires a char* given in the argument list.\n"
                      "[test.cpp:4]: (warning) %s in format string (no. 2) requires a char* given in the argument list.\n"
                      "[test.cpp:5]: (warning) %s in format string (no. 1) requires a char* given in the argument list.\n"
                      "[test.cpp:7]: (warning) %u in format string (no. 1) requires an unsigned integer but the argument type is 'char *'.\n", errout.str());

        check("void foo(const int* cpi, const int ci, int i, int* pi, std::string s) {\n"
              "    printf(\"%n\", cpi);\n"
              "    printf(\"%n\", ci);\n"
              "    printf(\"%n\", i);\n"
              "    printf(\"%n\", pi);\n"
              "    printf(\"%n\", s);\n"
              "    printf(\"%n\", \"s4\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) %n in format string (no. 1) requires a pointer to an non-const integer given in the argument list.\n"
                      "[test.cpp:3]: (warning) %n in format string (no. 1) requires a pointer to an non-const integer given in the argument list.\n"
                      "[test.cpp:4]: (warning) %n in format string (no. 1) requires a pointer to an non-const integer given in the argument list.\n"
                      "[test.cpp:6]: (warning) %n in format string (no. 1) requires a pointer to an non-const integer given in the argument list.\n"
                      "[test.cpp:7]: (warning) %n in format string (no. 1) requires a pointer to an non-const integer given in the argument list.\n", errout.str());

        check("class foo {};\n"
              "void foo(const int* cpi, foo f, bar b, bar* bp, double d, int i, unsigned int u) {\n"
              "    printf(\"%X\", f);\n"
              "    printf(\"%c\", \"s4\");\n"
              "    printf(\"%o\", d);\n"
              "    printf(\"%x\", cpi);\n"
              "    printf(\"%o\", b);\n"
              "    printf(\"%X\", bp);\n"
              "    printf(\"%X\", u);\n"
              "    printf(\"%X\", i);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) %X in format string (no. 1) requires an integer but the argument type is 'foo'.\n"
                      "[test.cpp:4]: (warning) %c in format string (no. 1) requires an integer but the argument type is 'const char *'.\n"
                      "[test.cpp:5]: (warning) %o in format string (no. 1) requires an integer but the argument type is 'double'.\n"
                      "[test.cpp:6]: (warning) %x in format string (no. 1) requires an integer but the argument type is 'int *'.\n"
                      "[test.cpp:8]: (warning) %X in format string (no. 1) requires an integer but the argument type is 'bar *'.\n", errout.str());

        check("class foo {};\n"
              "void foo(const int* cpi, foo f, bar b, bar* bp, double d, unsigned int u, unsigned char uc) {\n"
              "    printf(\"%i\", f);\n"
              "    printf(\"%d\", \"s4\");\n"
              "    printf(\"%d\", d);\n"
              "    printf(\"%d\", u);\n"
              "    printf(\"%d\", cpi);\n"
              "    printf(\"%i\", b);\n"
              "    printf(\"%i\", bp);\n"
              "    printf(\"%i\", uc);\n" // char is smaller than int, so there shouldn't be a problem
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) %i in format string (no. 1) requires a signed integer but the argument type is 'foo'.\n"
                      "[test.cpp:4]: (warning) %d in format string (no. 1) requires a signed integer but the argument type is 'const char *'.\n"
                      "[test.cpp:5]: (warning) %d in format string (no. 1) requires a signed integer but the argument type is 'double'.\n"
                      "[test.cpp:6]: (warning) %d in format string (no. 1) requires a signed integer but the argument type is 'unsigned int'.\n"
                      "[test.cpp:7]: (warning) %d in format string (no. 1) requires a signed integer but the argument type is 'int *'.\n"
                      "[test.cpp:9]: (warning) %i in format string (no. 1) requires a signed integer but the argument type is 'bar *'.\n", errout.str());

        check("class foo {};\n"
              "void foo(const int* cpi, foo f, bar b, bar* bp, double d, int i, bool bo) {\n"
              "    printf(\"%u\", f);\n"
              "    printf(\"%u\", \"s4\");\n"
              "    printf(\"%u\", d);\n"
              "    printf(\"%u\", i);\n"
              "    printf(\"%u\", cpi);\n"
              "    printf(\"%u\", b);\n"
              "    printf(\"%u\", bp);\n"
              "    printf(\"%u\", bo);\n" // bool shouldn't have a negative sign
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) %u in format string (no. 1) requires an unsigned integer but the argument type is 'foo'.\n"
                      "[test.cpp:4]: (warning) %u in format string (no. 1) requires an unsigned integer but the argument type is 'const char *'.\n"
                      "[test.cpp:5]: (warning) %u in format string (no. 1) requires an unsigned integer but the argument type is 'double'.\n"
                      "[test.cpp:6]: (warning) %u in format string (no. 1) requires an unsigned integer but the argument type is 'int'.\n"
                      "[test.cpp:7]: (warning) %u in format string (no. 1) requires an unsigned integer but the argument type is 'int *'.\n"
                      "[test.cpp:9]: (warning) %u in format string (no. 1) requires an unsigned integer but the argument type is 'bar *'.\n", errout.str());

        check("class foo {};\n"
              "void foo(const int* cpi, foo f, bar b, bar* bp, char c) {\n"
              "    printf(\"%p\", f);\n"
              "    printf(\"%p\", c);\n"
              "    printf(\"%p\", bp);\n"
              "    printf(\"%p\", cpi);\n"
              "    printf(\"%p\", b);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) %p in format string (no. 1) requires an address but the argument type is 'foo'.\n"
                      "[test.cpp:4]: (warning) %p in format string (no. 1) requires an address but the argument type is 'char'.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:3]: (warning) %e in format string (no. 1) requires a floating point number but the argument type is 'foo'.\n"
                      "[test.cpp:4]: (warning) %E in format string (no. 1) requires a floating point number but the argument type is 'const char *'.\n"
                      "[test.cpp:5]: (warning) %f in format string (no. 1) requires a floating point number but the argument type is 'int *'.\n"
                      "[test.cpp:6]: (warning) %G in format string (no. 1) requires a floating point number but the argument type is 'bar *'.\n", errout.str());

        check("class foo;\n"
              "void foo(foo f) {\n"
              "    printf(\"%u\", f);\n"
              "    printf(\"%f\", f);\n"
              "    printf(\"%p\", f);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) %u in format string (no. 1) requires an unsigned integer but the argument type is 'foo'.\n"
                      "[test.cpp:4]: (warning) %f in format string (no. 1) requires a floating point number but the argument type is 'foo'.\n"
                      "[test.cpp:5]: (warning) %p in format string (no. 1) requires an address but the argument type is 'foo'.\n", errout.str());

        // Ticket #4189 (Improve check (printf("%l") not detected)) tests (according to C99 7.19.6.1.7)
        // False positive tests
        check("void foo(signed char sc, unsigned char uc, short int si, unsigned short int usi) {\n"
              "  printf(\"%hhx %hhd\", sc, uc);\n"
              "  printf(\"%hd %hu\", si, usi);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo(long long int lli, unsigned long long int ulli, long int li, unsigned long int uli) {\n"
              "  printf(\"%llo %llx\", lli, ulli);\n"
              "  printf(\"%ld %lu\", li, uli);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo(intmax_t im, uintmax_t uim, size_t s, ptrdiff_t p, long double ld) {\n"
              "  printf(\"%jd %jo\", im, uim);\n"
              "  printf(\"%zx\", s);\n"
              "  printf(\"%ti\", p);\n"
              "  printf(\"%La\", ld);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // False negative test
        check("void foo(unsigned int i) {\n"
              "  printf(\"%h\", i);\n"
              "  printf(\"%hh\", i);\n"
              "  printf(\"%l\", i);\n"
              "  printf(\"%ll\", i);\n"
              "  printf(\"%j\", i);\n"
              "  printf(\"%z\", i);\n"
              "  printf(\"%t\", i);\n"
              "  printf(\"%L\", i);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) 'h' in format string (no. 1) is a length modifier and cannot be used without a conversion specifier.\n"
                      "[test.cpp:3]: (warning) 'hh' in format string (no. 1) is a length modifier and cannot be used without a conversion specifier.\n"
                      "[test.cpp:4]: (warning) 'l' in format string (no. 1) is a length modifier and cannot be used without a conversion specifier.\n"
                      "[test.cpp:5]: (warning) 'll' in format string (no. 1) is a length modifier and cannot be used without a conversion specifier.\n"
                      "[test.cpp:6]: (warning) 'j' in format string (no. 1) is a length modifier and cannot be used without a conversion specifier.\n"
                      "[test.cpp:7]: (warning) 'z' in format string (no. 1) is a length modifier and cannot be used without a conversion specifier.\n"
                      "[test.cpp:8]: (warning) 't' in format string (no. 1) is a length modifier and cannot be used without a conversion specifier.\n"
                      "[test.cpp:9]: (warning) 'L' in format string (no. 1) is a length modifier and cannot be used without a conversion specifier.\n", errout.str());

        check("void foo(unsigned int i) {\n"
              "  printf(\"%hd\", i);\n"
              "  printf(\"%hhd\", i);\n"
              "  printf(\"%ld\", i);\n"
              "  printf(\"%lld\", i);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) %hd in format string (no. 1) requires a signed integer but the argument type is 'unsigned int'.\n"
                      "[test.cpp:3]: (warning) %hhd in format string (no. 1) requires a signed integer but the argument type is 'unsigned int'.\n"
                      "[test.cpp:4]: (warning) %ld in format string (no. 1) requires a signed long integer but the argument type is 'unsigned int'.\n"
                      "[test.cpp:5]: (warning) %lld in format string (no. 1) requires a signed long long integer but the argument type is 'unsigned int'.\n" , errout.str());

        check("void foo(size_t s, ptrdiff_t p) {\n"
              "  printf(\"%zd\", s);\n"
              "  printf(\"%tu\", p);\n"
              "}", false, false, Settings::Unix32);
        ASSERT_EQUALS("[test.cpp:2]: (warning) %zd in format string (no. 1) requires a signed integer but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:3]: (warning) %tu in format string (no. 1) requires an unsigned integer but the argument type is 'ptrdiff_t {aka long}'.\n", errout.str());

        check("void foo(size_t s, ptrdiff_t p) {\n"
              "  printf(\"%zd\", s);\n"
              "  printf(\"%tu\", p);\n"
              "}", false, false, Settings::Unix64);
        ASSERT_EQUALS("[test.cpp:2]: (warning) %zd in format string (no. 1) requires a signed integer but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:3]: (warning) %tu in format string (no. 1) requires an unsigned integer but the argument type is 'ptrdiff_t {aka long}'.\n", errout.str());

        check("void foo(size_t s, ptrdiff_t p) {\n"
              "  printf(\"%zd\", s);\n"
              "  printf(\"%tu\", p);\n"
              "}", false, false, Settings::Win32A);
        ASSERT_EQUALS("[test.cpp:2]: (warning) %zd in format string (no. 1) requires a signed integer but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:3]: (warning) %tu in format string (no. 1) requires an unsigned integer but the argument type is 'ptrdiff_t {aka long}'.\n", errout.str());

        check("void foo(size_t s, ptrdiff_t p) {\n"
              "  printf(\"%zd\", s);\n"
              "  printf(\"%tu\", p);\n"
              "}", false, false, Settings::Win64);
        ASSERT_EQUALS("[test.cpp:2]: (warning) %zd in format string (no. 1) requires a signed integer but the argument type is 'size_t {aka unsigned long long}'.\n"
                      "[test.cpp:3]: (warning) %tu in format string (no. 1) requires an unsigned integer but the argument type is 'ptrdiff_t {aka long long}'.\n", errout.str());

        check("void foo(unsigned int i) {\n"
              "  printf(\"%ld\", i);\n"
              "  printf(\"%lld\", i);\n"
              "  printf(\"%lu\", i);\n"
              "  printf(\"%llu\", i);\n"
              "  printf(\"%lx\", i);\n"
              "  printf(\"%llx\", i);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) %ld in format string (no. 1) requires a signed long integer but the argument type is 'unsigned int'.\n"
                      "[test.cpp:3]: (warning) %lld in format string (no. 1) requires a signed long long integer but the argument type is 'unsigned int'.\n"
                      "[test.cpp:4]: (warning) %lu in format string (no. 1) requires an unsigned long integer but the argument type is 'unsigned int'.\n"
                      "[test.cpp:5]: (warning) %llu in format string (no. 1) requires an unsigned long long integer but the argument type is 'unsigned int'.\n"
                      "[test.cpp:6]: (warning) %lx in format string (no. 1) requires a long integer but the argument type is 'unsigned int'.\n"
                      "[test.cpp:7]: (warning) %llx in format string (no. 1) requires a long long integer but the argument type is 'unsigned int'.\n", errout.str());

        check("class Foo {\n"
              "    double d;\n"
              "    struct Bar {\n"
              "        int i;\n"
              "    } bar[2];\n"
              "    struct Baz {\n"
              "        int i;\n"
              "    } baz;\n"
              "};\n"
              "int a[10];\n"
              "Foo f[10];\n"
              "void foo(const Foo* foo) {\n"
              "    printf(\"%d %f %f %d %f %f\",\n"
              "        foo->d, foo->bar[0].i, a[0],\n"
              "        f[0].d, f[0].baz.i, f[0].bar[0].i);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:13]: (warning) %d in format string (no. 1) requires a signed integer but the argument type is 'double'.\n"
                      "[test.cpp:13]: (warning) %f in format string (no. 2) requires a floating point number but the argument type is 'int'.\n"
                      "[test.cpp:13]: (warning) %f in format string (no. 3) requires a floating point number but the argument type is 'int'.\n"
                      "[test.cpp:13]: (warning) %d in format string (no. 4) requires a signed integer but the argument type is 'double'.\n"
                      "[test.cpp:13]: (warning) %f in format string (no. 5) requires a floating point number but the argument type is 'int'.\n"
                      "[test.cpp:13]: (warning) %f in format string (no. 6) requires a floating point number but the argument type is 'int'.\n", errout.str());

        check("short f() { return 0; }\n"
              "void foo() { printf(\"%d %u %lu %I64u %I64d %f %lf %p\", f(), f(), f(), f(), f(), f(), f(), f()); }");
        ASSERT_EQUALS("[test.cpp:2]: (warning) %u in format string (no. 2) requires an unsigned integer but the argument type is 'short'.\n"
                      "[test.cpp:2]: (warning) %lu in format string (no. 3) requires an unsigned long integer but the argument type is 'short'.\n"
                      "[test.cpp:2]: (warning) %I64u in format string (no. 4) requires an unsigned long long integer but the argument type is 'short'.\n"
                      "[test.cpp:2]: (warning) %I64d in format string (no. 5) requires a signed long long integer but the argument type is 'short'.\n"
                      "[test.cpp:2]: (warning) %f in format string (no. 6) requires a floating point number but the argument type is 'short'.\n"
                      "[test.cpp:2]: (warning) %lf in format string (no. 7) requires a floating point number but the argument type is 'short'.\n"
                      "[test.cpp:2]: (warning) %p in format string (no. 8) requires an address but the argument type is 'short'.\n", errout.str());

        check("unsigned short f() { return 0; }\n"
              "void foo() { printf(\"%u %d %ld %I64d %I64u %f %lf %p\", f(), f(), f(), f(), f(), f(), f(), f()); }");
        ASSERT_EQUALS("[test.cpp:2]: (warning) %ld in format string (no. 3) requires a signed long integer but the argument type is 'unsigned short'.\n"
                      "[test.cpp:2]: (warning) %I64d in format string (no. 4) requires a signed long long integer but the argument type is 'unsigned short'.\n"
                      "[test.cpp:2]: (warning) %I64u in format string (no. 5) requires an unsigned long long integer but the argument type is 'unsigned short'.\n"
                      "[test.cpp:2]: (warning) %f in format string (no. 6) requires a floating point number but the argument type is 'unsigned short'.\n"
                      "[test.cpp:2]: (warning) %lf in format string (no. 7) requires a floating point number but the argument type is 'unsigned short'.\n"
                      "[test.cpp:2]: (warning) %p in format string (no. 8) requires an address but the argument type is 'unsigned short'.\n", errout.str());

        check("int f() { return 0; }\n"
              "void foo() { printf(\"%d %u %lu %I64u %I64d %f %lf %p\", f(), f(), f(), f(), f(), f(), f(), f()); }");
        ASSERT_EQUALS("[test.cpp:2]: (warning) %u in format string (no. 2) requires an unsigned integer but the argument type is 'int'.\n"
                      "[test.cpp:2]: (warning) %lu in format string (no. 3) requires an unsigned long integer but the argument type is 'int'.\n"
                      "[test.cpp:2]: (warning) %I64u in format string (no. 4) requires an unsigned long long integer but the argument type is 'int'.\n"
                      "[test.cpp:2]: (warning) %I64d in format string (no. 5) requires a signed long long integer but the argument type is 'int'.\n"
                      "[test.cpp:2]: (warning) %f in format string (no. 6) requires a floating point number but the argument type is 'int'.\n"
                      "[test.cpp:2]: (warning) %lf in format string (no. 7) requires a floating point number but the argument type is 'int'.\n"
                      "[test.cpp:2]: (warning) %p in format string (no. 8) requires an address but the argument type is 'int'.\n", errout.str());

        check("unsigned int f() { return 0; }\n"
              "void foo() { printf(\"%u %d %ld %I64d %I64u %f %lf %p\", f(), f(), f(), f(), f(), f(), f(), f()); }");
        ASSERT_EQUALS("[test.cpp:2]: (warning) %d in format string (no. 2) requires a signed integer but the argument type is 'unsigned int'.\n"
                      "[test.cpp:2]: (warning) %ld in format string (no. 3) requires a signed long integer but the argument type is 'unsigned int'.\n"
                      "[test.cpp:2]: (warning) %I64d in format string (no. 4) requires a signed long long integer but the argument type is 'unsigned int'.\n"
                      "[test.cpp:2]: (warning) %I64u in format string (no. 5) requires an unsigned long long integer but the argument type is 'unsigned int'.\n"
                      "[test.cpp:2]: (warning) %f in format string (no. 6) requires a floating point number but the argument type is 'unsigned int'.\n"
                      "[test.cpp:2]: (warning) %lf in format string (no. 7) requires a floating point number but the argument type is 'unsigned int'.\n"
                      "[test.cpp:2]: (warning) %p in format string (no. 8) requires an address but the argument type is 'unsigned int'.\n", errout.str());

        check("long f() { return 0; }\n"
              "void foo() { printf(\"%ld %u %lu %I64u %I64d %f %lf %p\", f(), f(), f(), f(), f(), f(), f(), f()); }");
        ASSERT_EQUALS("[test.cpp:2]: (warning) %u in format string (no. 2) requires an unsigned integer but the argument type is 'long'.\n"
                      "[test.cpp:2]: (warning) %lu in format string (no. 3) requires an unsigned long integer but the argument type is 'long'.\n"
                      "[test.cpp:2]: (warning) %I64u in format string (no. 4) requires an unsigned long long integer but the argument type is 'long'.\n"
                      "[test.cpp:2]: (warning) %I64d in format string (no. 5) requires a signed long long integer but the argument type is 'long'.\n"
                      "[test.cpp:2]: (warning) %f in format string (no. 6) requires a floating point number but the argument type is 'long'.\n"
                      "[test.cpp:2]: (warning) %lf in format string (no. 7) requires a floating point number but the argument type is 'long'.\n"
                      "[test.cpp:2]: (warning) %p in format string (no. 8) requires an address but the argument type is 'long'.\n", errout.str());

        check("unsigned long f() { return 0; }\n"
              "void foo() { printf(\"%lu %d %ld %I64d %I64u %f %lf %p\", f(), f(), f(), f(), f(), f(), f(), f()); }");
        ASSERT_EQUALS("[test.cpp:2]: (warning) %d in format string (no. 2) requires a signed integer but the argument type is 'unsigned long'.\n"
                      "[test.cpp:2]: (warning) %ld in format string (no. 3) requires a signed long integer but the argument type is 'unsigned long'.\n"
                      "[test.cpp:2]: (warning) %I64d in format string (no. 4) requires a signed long long integer but the argument type is 'unsigned long'.\n"
                      "[test.cpp:2]: (warning) %I64u in format string (no. 5) requires an unsigned long long integer but the argument type is 'unsigned long'.\n"
                      "[test.cpp:2]: (warning) %f in format string (no. 6) requires a floating point number but the argument type is 'unsigned long'.\n"
                      "[test.cpp:2]: (warning) %lf in format string (no. 7) requires a floating point number but the argument type is 'unsigned long'.\n"
                      "[test.cpp:2]: (warning) %p in format string (no. 8) requires an address but the argument type is 'unsigned long'.\n", errout.str());

        check("long long f() { return 0; }\n"
              "void foo() { printf(\"%lld %u %lu %I64u %I64d %f %lf %p\", f(), f(), f(), f(), f(), f(), f(), f()); }");
        ASSERT_EQUALS("[test.cpp:2]: (warning) %u in format string (no. 2) requires an unsigned integer but the argument type is 'long long'.\n"
                      "[test.cpp:2]: (warning) %lu in format string (no. 3) requires an unsigned long integer but the argument type is 'long long'.\n"
                      "[test.cpp:2]: (warning) %I64u in format string (no. 4) requires an unsigned long long integer but the argument type is 'long long'.\n"
                      "[test.cpp:2]: (warning) %f in format string (no. 6) requires a floating point number but the argument type is 'long long'.\n"
                      "[test.cpp:2]: (warning) %lf in format string (no. 7) requires a floating point number but the argument type is 'long long'.\n"
                      "[test.cpp:2]: (warning) %p in format string (no. 8) requires an address but the argument type is 'long long'.\n", errout.str());

        check("unsigned long long f() { return 0; }\n"
              "void foo() { printf(\"%llu %d %ld %I64d %I64u %f %lf %p\", f(), f(), f(), f(), f(), f(), f(), f()); }");
        ASSERT_EQUALS("[test.cpp:2]: (warning) %d in format string (no. 2) requires a signed integer but the argument type is 'unsigned long long'.\n"
                      "[test.cpp:2]: (warning) %ld in format string (no. 3) requires a signed long integer but the argument type is 'unsigned long long'.\n"
                      "[test.cpp:2]: (warning) %I64d in format string (no. 4) requires a signed long long integer but the argument type is 'unsigned long long'.\n"
                      "[test.cpp:2]: (warning) %f in format string (no. 6) requires a floating point number but the argument type is 'unsigned long long'.\n"
                      "[test.cpp:2]: (warning) %lf in format string (no. 7) requires a floating point number but the argument type is 'unsigned long long'.\n"
                      "[test.cpp:2]: (warning) %p in format string (no. 8) requires an address but the argument type is 'unsigned long long'.\n", errout.str());

        check("float f() { return 0; }\n"
              "void foo() { printf(\"%f %d %ld %u %lu %I64d %I64u %lf %p\", f(), f(), f(), f(), f(), f(), f(), f(), f()); }");
        ASSERT_EQUALS("[test.cpp:2]: (warning) %d in format string (no. 2) requires a signed integer but the argument type is 'float'.\n"
                      "[test.cpp:2]: (warning) %ld in format string (no. 3) requires a signed long integer but the argument type is 'float'.\n"
                      "[test.cpp:2]: (warning) %u in format string (no. 4) requires an unsigned integer but the argument type is 'float'.\n"
                      "[test.cpp:2]: (warning) %lu in format string (no. 5) requires an unsigned long integer but the argument type is 'float'.\n"
                      "[test.cpp:2]: (warning) %I64d in format string (no. 6) requires a signed long long integer but the argument type is 'float'.\n"
                      "[test.cpp:2]: (warning) %I64u in format string (no. 7) requires an unsigned long long integer but the argument type is 'float'.\n"
                      "[test.cpp:2]: (warning) %lf in format string (no. 8) requires a floating point number but the argument type is 'float'.\n"
                      "[test.cpp:2]: (warning) %p in format string (no. 9) requires an address but the argument type is 'float'.\n", errout.str());

        check("double f() { return 0; }\n"
              "void foo() { printf(\"%f %d %ld %u %lu %I64d %I64u %lf %p\", f(), f(), f(), f(), f(), f(), f(), f(), f()); }");
        ASSERT_EQUALS("[test.cpp:2]: (warning) %d in format string (no. 2) requires a signed integer but the argument type is 'double'.\n"
                      "[test.cpp:2]: (warning) %ld in format string (no. 3) requires a signed long integer but the argument type is 'double'.\n"
                      "[test.cpp:2]: (warning) %u in format string (no. 4) requires an unsigned integer but the argument type is 'double'.\n"
                      "[test.cpp:2]: (warning) %lu in format string (no. 5) requires an unsigned long integer but the argument type is 'double'.\n"
                      "[test.cpp:2]: (warning) %I64d in format string (no. 6) requires a signed long long integer but the argument type is 'double'.\n"
                      "[test.cpp:2]: (warning) %I64u in format string (no. 7) requires an unsigned long long integer but the argument type is 'double'.\n"
                      "[test.cpp:2]: (warning) %lf in format string (no. 8) requires a floating point number but the argument type is 'double'.\n"
                      "[test.cpp:2]: (warning) %p in format string (no. 9) requires an address but the argument type is 'double'.\n", errout.str());

        check("long double f() { return 0; }\n"
              "void foo() { printf(\"%lf %d %ld %u %lu %I64d %I64u %f %p\", f(), f(), f(), f(), f(), f(), f(), f(), f()); }");
        ASSERT_EQUALS("[test.cpp:2]: (warning) %d in format string (no. 2) requires a signed integer but the argument type is 'long double'.\n"
                      "[test.cpp:2]: (warning) %ld in format string (no. 3) requires a signed long integer but the argument type is 'long double'.\n"
                      "[test.cpp:2]: (warning) %u in format string (no. 4) requires an unsigned integer but the argument type is 'long double'.\n"
                      "[test.cpp:2]: (warning) %lu in format string (no. 5) requires an unsigned long integer but the argument type is 'long double'.\n"
                      "[test.cpp:2]: (warning) %I64d in format string (no. 6) requires a signed long long integer but the argument type is 'long double'.\n"
                      "[test.cpp:2]: (warning) %I64u in format string (no. 7) requires an unsigned long long integer but the argument type is 'long double'.\n"
                      "[test.cpp:2]: (warning) %f in format string (no. 8) requires a floating point number but the argument type is 'long double'.\n"
                      "[test.cpp:2]: (warning) %p in format string (no. 9) requires an address but the argument type is 'long double'.\n", errout.str());

        check("namespace bar { int f() { return 0; } }\n"
              "void foo() { printf(\"%d %u %lu %f %lf %p\", bar::f(), bar::f(), bar::f(), bar::f(), bar::f(), bar::f()); }");
        ASSERT_EQUALS("[test.cpp:2]: (warning) %u in format string (no. 2) requires an unsigned integer but the argument type is 'int'.\n"
                      "[test.cpp:2]: (warning) %lu in format string (no. 3) requires an unsigned long integer but the argument type is 'int'.\n"
                      "[test.cpp:2]: (warning) %f in format string (no. 4) requires a floating point number but the argument type is 'int'.\n"
                      "[test.cpp:2]: (warning) %lf in format string (no. 5) requires a floating point number but the argument type is 'int'.\n"
                      "[test.cpp:2]: (warning) %p in format string (no. 6) requires an address but the argument type is 'int'.\n", errout.str());

        check("struct Fred { int i; } f;\n"
              "void foo() { printf(\"%d %u %lu %f %lf %p\", f.i, f.i, f.i, f.i, f.i, f.i); }");
        ASSERT_EQUALS("[test.cpp:2]: (warning) %u in format string (no. 2) requires an unsigned integer but the argument type is 'int'.\n"
                      "[test.cpp:2]: (warning) %lu in format string (no. 3) requires an unsigned long integer but the argument type is 'int'.\n"
                      "[test.cpp:2]: (warning) %f in format string (no. 4) requires a floating point number but the argument type is 'int'.\n"
                      "[test.cpp:2]: (warning) %lf in format string (no. 5) requires a floating point number but the argument type is 'int'.\n"
                      "[test.cpp:2]: (warning) %p in format string (no. 6) requires an address but the argument type is 'int'.\n", errout.str());

        check("struct Fred { unsigned int u; } f;\n"
              "void foo() { printf(\"%u %d %ld %f %lf %p\", f.u, f.u, f.u, f.u, f.u, f.u); }");
        ASSERT_EQUALS("[test.cpp:2]: (warning) %d in format string (no. 2) requires a signed integer but the argument type is 'unsigned int'.\n"
                      "[test.cpp:2]: (warning) %ld in format string (no. 3) requires a signed long integer but the argument type is 'unsigned int'.\n"
                      "[test.cpp:2]: (warning) %f in format string (no. 4) requires a floating point number but the argument type is 'unsigned int'.\n"
                      "[test.cpp:2]: (warning) %lf in format string (no. 5) requires a floating point number but the argument type is 'unsigned int'.\n"
                      "[test.cpp:2]: (warning) %p in format string (no. 6) requires an address but the argument type is 'unsigned int'.\n", errout.str());

        check("struct Fred { unsigned int ui() { return 0; } } f;\n"
              "void foo() { printf(\"%u %d %ld %f %lf %p\", f.ui(), f.ui(), f.ui(), f.ui(), f.ui(), f.ui()); }");
        ASSERT_EQUALS("[test.cpp:2]: (warning) %d in format string (no. 2) requires a signed integer but the argument type is 'unsigned int'.\n"
                      "[test.cpp:2]: (warning) %ld in format string (no. 3) requires a signed long integer but the argument type is 'unsigned int'.\n"
                      "[test.cpp:2]: (warning) %f in format string (no. 4) requires a floating point number but the argument type is 'unsigned int'.\n"
                      "[test.cpp:2]: (warning) %lf in format string (no. 5) requires a floating point number but the argument type is 'unsigned int'.\n"
                      "[test.cpp:2]: (warning) %p in format string (no. 6) requires an address but the argument type is 'unsigned int'.\n", errout.str());

        // #4975
        check("void f(int len, int newline) {\n"
              "    printf(\"%s\", newline ? a : str + len);\n"
              "    printf(\"%s\", newline + newline);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("struct Fred { int i; } f;\n"
              "struct Fred & bar() { };\n"
              "void foo() { printf(\"%d %u %lu %f %lf %p\", bar().i, bar().i, bar().i, bar().i, bar().i, bar().i); }");
        ASSERT_EQUALS("[test.cpp:3]: (warning) %u in format string (no. 2) requires an unsigned integer but the argument type is 'int'.\n"
                      "[test.cpp:3]: (warning) %lu in format string (no. 3) requires an unsigned long integer but the argument type is 'int'.\n"
                      "[test.cpp:3]: (warning) %f in format string (no. 4) requires a floating point number but the argument type is 'int'.\n"
                      "[test.cpp:3]: (warning) %lf in format string (no. 5) requires a floating point number but the argument type is 'int'.\n"
                      "[test.cpp:3]: (warning) %p in format string (no. 6) requires an address but the argument type is 'int'.\n", errout.str());

        check("struct Fred { int i; } f;\n"
              "const struct Fred & bar() { };\n"
              "void foo() { printf(\"%d %u %lu %f %lf %p\", bar().i, bar().i, bar().i, bar().i, bar().i, bar().i); }");
        ASSERT_EQUALS("[test.cpp:3]: (warning) %u in format string (no. 2) requires an unsigned integer but the argument type is 'int'.\n"
                      "[test.cpp:3]: (warning) %lu in format string (no. 3) requires an unsigned long integer but the argument type is 'int'.\n"
                      "[test.cpp:3]: (warning) %f in format string (no. 4) requires a floating point number but the argument type is 'int'.\n"
                      "[test.cpp:3]: (warning) %lf in format string (no. 5) requires a floating point number but the argument type is 'int'.\n"
                      "[test.cpp:3]: (warning) %p in format string (no. 6) requires an address but the argument type is 'int'.\n", errout.str());

        check("struct Fred { int i; } f;\n"
              "static const struct Fred & bar() { };\n"
              "void foo() { printf(\"%d %u %lu %f %lf %p\", bar().i, bar().i, bar().i, bar().i, bar().i, bar().i); }");
        ASSERT_EQUALS("[test.cpp:3]: (warning) %u in format string (no. 2) requires an unsigned integer but the argument type is 'int'.\n"
                      "[test.cpp:3]: (warning) %lu in format string (no. 3) requires an unsigned long integer but the argument type is 'int'.\n"
                      "[test.cpp:3]: (warning) %f in format string (no. 4) requires a floating point number but the argument type is 'int'.\n"
                      "[test.cpp:3]: (warning) %lf in format string (no. 5) requires a floating point number but the argument type is 'int'.\n"
                      "[test.cpp:3]: (warning) %p in format string (no. 6) requires an address but the argument type is 'int'.\n", errout.str());

        check("struct Fred { int i; } f[2];\n"
              "struct Fred * bar() { return f; };\n"
              "void foo() { printf(\"%d %u %lu %f %lf %p\", bar()[0].i, bar()[0].i, bar()[0].i, bar()[0].i, bar()[0].i, bar()[0].i); }");
        ASSERT_EQUALS("[test.cpp:3]: (warning) %u in format string (no. 2) requires an unsigned integer but the argument type is 'int'.\n"
                      "[test.cpp:3]: (warning) %lu in format string (no. 3) requires an unsigned long integer but the argument type is 'int'.\n"
                      "[test.cpp:3]: (warning) %f in format string (no. 4) requires a floating point number but the argument type is 'int'.\n"
                      "[test.cpp:3]: (warning) %lf in format string (no. 5) requires a floating point number but the argument type is 'int'.\n"
                      "[test.cpp:3]: (warning) %p in format string (no. 6) requires an address but the argument type is 'int'.\n", errout.str());

        check("struct Fred { int i; } f[2];\n"
              "const struct Fred * bar() { return f; };\n"
              "void foo() { printf(\"%d %u %lu %f %lf %p\", bar()[0].i, bar()[0].i, bar()[0].i, bar()[0].i, bar()[0].i, bar()[0].i); }");
        ASSERT_EQUALS("[test.cpp:3]: (warning) %u in format string (no. 2) requires an unsigned integer but the argument type is 'int'.\n"
                      "[test.cpp:3]: (warning) %lu in format string (no. 3) requires an unsigned long integer but the argument type is 'int'.\n"
                      "[test.cpp:3]: (warning) %f in format string (no. 4) requires a floating point number but the argument type is 'int'.\n"
                      "[test.cpp:3]: (warning) %lf in format string (no. 5) requires a floating point number but the argument type is 'int'.\n"
                      "[test.cpp:3]: (warning) %p in format string (no. 6) requires an address but the argument type is 'int'.\n", errout.str());

        check("struct Fred { int i; } f[2];\n"
              "static const struct Fred * bar() { return f; };\n"
              "void foo() { printf(\"%d %u %lu %f %lf %p\", bar()[0].i, bar()[0].i, bar()[0].i, bar()[0].i, bar()[0].i, bar()[0].i); }");
        ASSERT_EQUALS("[test.cpp:3]: (warning) %u in format string (no. 2) requires an unsigned integer but the argument type is 'int'.\n"
                      "[test.cpp:3]: (warning) %lu in format string (no. 3) requires an unsigned long integer but the argument type is 'int'.\n"
                      "[test.cpp:3]: (warning) %f in format string (no. 4) requires a floating point number but the argument type is 'int'.\n"
                      "[test.cpp:3]: (warning) %lf in format string (no. 5) requires a floating point number but the argument type is 'int'.\n"
                      "[test.cpp:3]: (warning) %p in format string (no. 6) requires an address but the argument type is 'int'.\n", errout.str());

        check("struct Fred { int32_t i; } f;\n"
              "struct Fred & bar() { };\n"
              "void foo() { printf(\"%d %ld %u %lu %f %lf\", bar().i, bar().i, bar().i, bar().i, bar().i, bar().i); }");
        ASSERT_EQUALS("", errout.str());

        // #4984
        check("void f(double *x) {\n"
              "    printf(\"%f\", x[0]);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("int array[10];\n"
              "int * foo() { return array; }\n"
              "void f() {\n"
              "    printf(\"%f\", foo()[0]);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (warning) %f in format string (no. 1) requires a floating point number but the argument type is 'int'.\n", errout.str());

        check("struct Base { int length() { } };\n"
              "struct Derived : public Base { };\n"
              "void foo(Derived * d) {\n"
              "    printf(\"%f\", d.length());\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (warning) %f in format string (no. 1) requires a floating point number but the argument type is 'int'.\n", errout.str());

        check("std::vector<int> v;\n"
              "void foo() {\n"
              "    printf(\"%d %u %f\", v[0], v[0], v[0]);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (warning) %u in format string (no. 2) requires an unsigned integer but the argument type is 'int'.\n"
                      "[test.cpp:3]: (warning) %f in format string (no. 3) requires a floating point number but the argument type is 'int'.\n", errout.str());

        // #4999 (crash)
        check("int bar(int a);\n"
              "void foo() {\n"
              "    printf(\"%d\", bar(0));\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("std::vector<int> v;\n"
              "std::string s;\n"
              "void foo() {\n"
              "    printf(\"%zu %Iu %d %f\", v.size(), v.size(), v.size(), v.size());\n"
              "    printf(\"%zu %Iu %d %f\", s.size(), s.size(), s.size(), s.size());\n"
              "}\n", false, false, Settings::Win32A);
        ASSERT_EQUALS("[test.cpp:4]: (warning) %d in format string (no. 3) requires a signed integer but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:4]: (warning) %f in format string (no. 4) requires a floating point number but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:5]: (warning) %d in format string (no. 3) requires a signed integer but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:5]: (warning) %f in format string (no. 4) requires a floating point number but the argument type is 'size_t {aka unsigned long}'.\n", errout.str());

        check("std::vector<int> v;\n"
              "std::string s;\n"
              "void foo() {\n"
              "    printf(\"%zu %Iu %d %f\", v.size(), v.size(), v.size(), v.size());\n"
              "    printf(\"%zu %Iu %d %f\", s.size(), s.size(), s.size(), s.size());\n"
              "}\n", false, false, Settings::Win64);
        ASSERT_EQUALS("[test.cpp:4]: (warning) %d in format string (no. 3) requires a signed integer but the argument type is 'size_t {aka unsigned long long}'.\n"
                      "[test.cpp:4]: (warning) %f in format string (no. 4) requires a floating point number but the argument type is 'size_t {aka unsigned long long}'.\n"
                      "[test.cpp:5]: (warning) %d in format string (no. 3) requires a signed integer but the argument type is 'size_t {aka unsigned long long}'.\n"
                      "[test.cpp:5]: (warning) %f in format string (no. 4) requires a floating point number but the argument type is 'size_t {aka unsigned long long}'.\n", errout.str());

        check("std::vector<int> v;\n"
              "std::string s;\n"
              "void foo() {\n"
              "    printf(\"%zu %Iu %d %f\", v.size(), v.size(), v.size(), v.size());\n"
              "    printf(\"%zu %Iu %d %f\", s.size(), s.size(), s.size(), s.size());\n"
              "}\n", false, false, Settings::Unix32);
        ASSERT_EQUALS("[test.cpp:4]: (warning) %d in format string (no. 3) requires a signed integer but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:4]: (warning) %f in format string (no. 4) requires a floating point number but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:5]: (warning) %d in format string (no. 3) requires a signed integer but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:5]: (warning) %f in format string (no. 4) requires a floating point number but the argument type is 'size_t {aka unsigned long}'.\n", errout.str());

        check("std::vector<int> v;\n"
              "std::string s;\n"
              "void foo() {\n"
              "    printf(\"%zu %Iu %d %f\", v.size(), v.size(), v.size(), v.size());\n"
              "    printf(\"%zu %Iu %d %f\", s.size(), s.size(), s.size(), s.size());\n"
              "}\n", false, false, Settings::Unix64);
        ASSERT_EQUALS("[test.cpp:4]: (warning) %d in format string (no. 3) requires a signed integer but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:4]: (warning) %f in format string (no. 4) requires a floating point number but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:5]: (warning) %d in format string (no. 3) requires a signed integer but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:5]: (warning) %f in format string (no. 4) requires a floating point number but the argument type is 'size_t {aka unsigned long}'.\n", errout.str());

        check("class Fred : public std::vector<int> {} v;\n"
              "std::string s;\n"
              "void foo() {\n"
              "    printf(\"%zu %Iu %d %f\", v.size(), v.size(), v.size(), v.size());\n"
              "    printf(\"%zu %Iu %d %f\", s.size(), s.size(), s.size(), s.size());\n"
              "}\n", false, false, Settings::Unix64);
        ASSERT_EQUALS("[test.cpp:4]: (warning) %d in format string (no. 3) requires a signed integer but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:4]: (warning) %f in format string (no. 4) requires a floating point number but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:5]: (warning) %d in format string (no. 3) requires a signed integer but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:5]: (warning) %f in format string (no. 4) requires a floating point number but the argument type is 'size_t {aka unsigned long}'.\n", errout.str());

        check("class Fred : public std::vector<int> {} v;\n"
              "void foo() {\n"
              "    printf(\"%d %u %f\", v[0], v[0], v[0]);\n"
              "}\n", false, false, Settings::Unix64);
        ASSERT_EQUALS("[test.cpp:3]: (warning) %u in format string (no. 2) requires an unsigned integer but the argument type is 'int'.\n"
                      "[test.cpp:3]: (warning) %f in format string (no. 3) requires a floating point number but the argument type is 'int'.\n", errout.str());

        check("std::string s;\n"
              "void foo() {\n"
              "    printf(\"%s %p %u %d %f\", s.c_str(), s.c_str(), s.c_str(), s.c_str(), s.c_str());\n"
              "}\n", false, false, Settings::Unix64);
        ASSERT_EQUALS("[test.cpp:3]: (warning) %u in format string (no. 3) requires an unsigned integer but the argument type is 'const char *'.\n"
                      "[test.cpp:3]: (warning) %d in format string (no. 4) requires a signed integer but the argument type is 'const char *'.\n"
                      "[test.cpp:3]: (warning) %f in format string (no. 5) requires a floating point number but the argument type is 'const char *'.\n", errout.str());

        check("std::vector<int> array;\n"
              "char * p = 0;\n"
              "char q[] = \"abc\";\n"
              "char r[10] = { 0 };\n"
              "size_t s;\n"
              "void foo() {\n"
              "    printf(\"%zu %zu\", array.size(), s);\n"
              "    printf(\"%u %u %u\", p, q, r);\n"
              "    printf(\"%u %u\", array.size(), s);\n"
              "    printf(\"%lu %lu\", array.size(), s);\n"
              "    printf(\"%llu %llu\", array.size(), s);\n"
              "}\n", false, false, Settings::Unix64);
        ASSERT_EQUALS("[test.cpp:8]: (warning) %u in format string (no. 1) requires an unsigned integer but the argument type is 'char *'.\n"
                      "[test.cpp:8]: (warning) %u in format string (no. 2) requires an unsigned integer but the argument type is 'char *'.\n"
                      "[test.cpp:8]: (warning) %u in format string (no. 3) requires an unsigned integer but the argument type is 'char *'.\n"
                      "[test.cpp:9]: (warning) %u in format string (no. 1) requires an unsigned integer but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:9]: (warning) %u in format string (no. 2) requires an unsigned integer but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:10]: (warning) %lu in format string (no. 1) requires an unsigned long integer but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:10]: (warning) %lu in format string (no. 2) requires an unsigned long integer but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:11]: (warning) %llu in format string (no. 1) requires an unsigned long long integer but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:11]: (warning) %llu in format string (no. 2) requires an unsigned long long integer but the argument type is 'size_t {aka unsigned long}'.\n", errout.str());

        check("bool b; bool bf(){ return 0; }\n"
              "char c; char cf(){ return 0; }\n"
              "signed char sc; signed char scf(){ return 0; }\n"
              "unsigned char uc; unsigned char ucf(){ return 0; }\n"
              "short s; short sf(){ return 0; }\n"
              "unsigned short us; unsigned short usf(){ return 0; }\n"
              "size_t st; size_t stf(){ return 0; }\n"
              "ptrdiff_t pt; ptrdiff_t ptf(){ return 0; }\n"
              "char * pc; char * pcf(){ return 0; }\n"
              "char cl[] = \"123\";\n"
              "char ca[3];\n"
              "void foo() {\n"
              "    printf(\"%td %zd %d %d %d %d %d %d %d %d %d %d %d\", pt, pt, b, c, sc, uc, s, us, st, pt, pc, cl, ca);\n"
              "    printf(\"%ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld\", b, c, sc, uc, s, us, st, pt, pc, cl, ca);\n"
              "    printf(\"%td %zd %d %d %d %d %d %d %d %d %d\", ptf(), ptf(), bf(), cf(), scf(), ucf(), sf(), usf(), stf(), ptf(), pcf());\n"
              "    printf(\"%ld %ld %ld %ld %ld %ld %ld %ld %ld\", bf(), cf(), scf(), ucf(), sf(), usf(), stf(), ptf(), pcf());\n"
              "}\n", false, false, Settings::Unix64);
        ASSERT_EQUALS("[test.cpp:13]: (warning) %zd in format string (no. 2) requires a signed integer but the argument type is 'ptrdiff_t {aka long}'.\n"
                      "[test.cpp:13]: (warning) %d in format string (no. 9) requires a signed integer but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:13]: (warning) %d in format string (no. 10) requires a signed integer but the argument type is 'ptrdiff_t {aka long}'.\n"
                      "[test.cpp:13]: (warning) %d in format string (no. 11) requires a signed integer but the argument type is 'char *'.\n"
                      "[test.cpp:13]: (warning) %d in format string (no. 12) requires a signed integer but the argument type is 'char *'.\n"
                      "[test.cpp:13]: (warning) %d in format string (no. 13) requires a signed integer but the argument type is 'char *'.\n"
                      "[test.cpp:14]: (warning) %ld in format string (no. 1) requires a signed long integer but the argument type is 'bool'.\n"
                      "[test.cpp:14]: (warning) %ld in format string (no. 2) requires a signed long integer but the argument type is 'char'.\n"
                      "[test.cpp:14]: (warning) %ld in format string (no. 3) requires a signed long integer but the argument type is 'signed char'.\n"
                      "[test.cpp:14]: (warning) %ld in format string (no. 4) requires a signed long integer but the argument type is 'unsigned char'.\n"
                      "[test.cpp:14]: (warning) %ld in format string (no. 5) requires a signed long integer but the argument type is 'short'.\n"
                      "[test.cpp:14]: (warning) %ld in format string (no. 6) requires a signed long integer but the argument type is 'unsigned short'.\n"
                      "[test.cpp:14]: (warning) %ld in format string (no. 7) requires a signed long integer but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:14]: (warning) %ld in format string (no. 8) requires a signed long integer but the argument type is 'ptrdiff_t {aka long}'.\n"
                      "[test.cpp:14]: (warning) %ld in format string (no. 9) requires a signed long integer but the argument type is 'char *'.\n"
                      "[test.cpp:14]: (warning) %ld in format string (no. 10) requires a signed long integer but the argument type is 'char *'.\n"
                      "[test.cpp:14]: (warning) %ld in format string (no. 11) requires a signed long integer but the argument type is 'char *'.\n"
                      "[test.cpp:15]: (warning) %zd in format string (no. 2) requires a signed integer but the argument type is 'ptrdiff_t {aka long}'.\n"
                      "[test.cpp:15]: (warning) %d in format string (no. 9) requires a signed integer but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:15]: (warning) %d in format string (no. 10) requires a signed integer but the argument type is 'ptrdiff_t {aka long}'.\n"
                      "[test.cpp:15]: (warning) %d in format string (no. 11) requires a signed integer but the argument type is 'char *'.\n"
                      "[test.cpp:16]: (warning) %ld in format string (no. 1) requires a signed long integer but the argument type is 'bool'.\n"
                      "[test.cpp:16]: (warning) %ld in format string (no. 2) requires a signed long integer but the argument type is 'char'.\n"
                      "[test.cpp:16]: (warning) %ld in format string (no. 3) requires a signed long integer but the argument type is 'signed char'.\n"
                      "[test.cpp:16]: (warning) %ld in format string (no. 4) requires a signed long integer but the argument type is 'unsigned char'.\n"
                      "[test.cpp:16]: (warning) %ld in format string (no. 5) requires a signed long integer but the argument type is 'short'.\n"
                      "[test.cpp:16]: (warning) %ld in format string (no. 6) requires a signed long integer but the argument type is 'unsigned short'.\n"
                      "[test.cpp:16]: (warning) %ld in format string (no. 7) requires a signed long integer but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:16]: (warning) %ld in format string (no. 8) requires a signed long integer but the argument type is 'ptrdiff_t {aka long}'.\n"
                      "[test.cpp:16]: (warning) %ld in format string (no. 9) requires a signed long integer but the argument type is 'char *'.\n", errout.str());

        check("struct A {};\n"
              "class B : public std::vector<const int *> {} b;\n"
              "class C : public std::vector<const struct A *> {} c;\n"
              "std::string s;\n"
              "void foo() {\n"
              "    printf(\"%zu %u\", b.size(), b.size());\n"
              "    printf(\"%p %d\", b[0], b[0]);\n"
              "    printf(\"%p %d\", c[0], c[0]);\n"
              "    printf(\"%p %d\", s.c_str(), s.c_str());\n"
              "}\n", false, false, Settings::Unix64);
        ASSERT_EQUALS("[test.cpp:6]: (warning) %u in format string (no. 2) requires an unsigned integer but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:7]: (warning) %d in format string (no. 2) requires a signed integer but the argument type is 'const int *'.\n"
                      "[test.cpp:8]: (warning) %d in format string (no. 2) requires a signed integer but the argument type is 'const struct A *'.\n"
                      "[test.cpp:9]: (warning) %d in format string (no. 2) requires a signed integer but the argument type is 'const char *'.\n", errout.str());

        check("class A : public std::vector<std::string> {} a;\n"
              "class B : public std::string {} b;\n"
              "std::string s;\n"
              "void foo() {\n"
              "    printf(\"%p %d\", a[0].c_str(), a[0].c_str());\n"
              "    printf(\"%c %p\", b[0], b[0]);\n"
              "    printf(\"%c %p\", s[0], s[0]);\n"
              "}\n", false, false, Settings::Unix64);
        ASSERT_EQUALS("[test.cpp:5]: (warning) %d in format string (no. 2) requires a signed integer but the argument type is 'const char *'.\n"
                      "[test.cpp:6]: (warning) %p in format string (no. 2) requires an address but the argument type is 'char'.\n"
                      "[test.cpp:7]: (warning) %p in format string (no. 2) requires an address but the argument type is 'char'.\n", errout.str());

    }

    void testPosixPrintfScanfParameterPosition() { // #4900  - No support for parameters in format strings
        check("void foo() {"
              "  int bar;"
              "  printf(\"%1$d\", 1);"
              "  printf(\"%1$d, %d, %1$d\", 1, 2);"
              "  scanf(\"%1$d\", &bar);"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo() {\n"
              "  int bar;\n"
              "  printf(\"%1$d\");\n"
              "  printf(\"%1$d, %d, %4$d\", 1, 2, 3);\n"
              "  scanf(\"%2$d\", &bar);\n"
              "  printf(\"%0$f\", 0.0);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) printf format string has 1 parameters but only 0 are given.\n"
                      "[test.cpp:4]: (warning) printf: referencing parameter 4 while 3 arguments given\n"
                      "[test.cpp:5]: (warning) scanf: referencing parameter 2 while 1 arguments given\n"
                      "[test.cpp:6]: (warning) printf: parameter positions start at 1, not 0\n"
                      "", errout.str());
    }


    void testMicrosoftPrintfArgument() {
        check("void foo() {\n"
              "    size_t s;\n"
              "    ptrdiff_t p;\n"
              "    __int32 i32;\n"
              "    unsigned __int32 u32;\n"
              "    __int64 i64;\n"
              "    unsigned __int64 u64;\n"
              "    printf(\"%Id %Iu %Ix\", s, s, s);\n"
              "    printf(\"%Id %Iu %Ix\", p, p, p);\n"
              "    printf(\"%I32d %I32u %I32x\", i32, i32, i32);\n"
              "    printf(\"%I32d %I32u %I32x\", u32, u32, u32);\n"
              "    printf(\"%I64d %I64u %I64x\", i64, i64, i64);\n"
              "    printf(\"%I64d %I64u %I64x\", u64, u64, u64);\n"
              "}", false, false, Settings::Win32A);
        ASSERT_EQUALS("[test.cpp:8]: (warning) %Id in format string (no. 1) requires a signed integer but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:9]: (warning) %Iu in format string (no. 2) requires an unsigned integer but the argument type is 'ptrdiff_t {aka long}'.\n"
                      "[test.cpp:10]: (warning) %I32u in format string (no. 2) requires an unsigned integer but the argument type is '__int32 {aka int}'.\n"
                      "[test.cpp:11]: (warning) %I32d in format string (no. 1) requires a signed integer but the argument type is 'unsigned __int32 {aka unsigned int}'.\n"
                      "[test.cpp:12]: (warning) %I64u in format string (no. 2) requires an unsigned long long integer but the argument type is '__int64 {aka long long}'.\n"
                      "[test.cpp:13]: (warning) %I64d in format string (no. 1) requires a signed long long integer but the argument type is 'unsigned __int64 {aka unsigned long long}'.\n", errout.str());

        check("void foo() {\n"
              "    size_t s;\n"
              "    ptrdiff_t p;\n"
              "    __int32 i32;\n"
              "    unsigned __int32 u32;\n"
              "    __int64 i64;\n"
              "    unsigned __int64 u64;\n"
              "    printf(\"%Id %Iu %Ix\", s, s, s);\n"
              "    printf(\"%Id %Iu %Ix\", p, p, p);\n"
              "    printf(\"%I32d %I32u %I32x\", i32, i32, i32);\n"
              "    printf(\"%I32d %I32u %I32x\", u32, u32, u32);\n"
              "    printf(\"%I64d %I64u %I64x\", i64, i64, i64);\n"
              "    printf(\"%I64d %I64u %I64x\", u64, u64, u64);\n"
              "}", false, false, Settings::Win64);
        ASSERT_EQUALS("[test.cpp:8]: (warning) %Id in format string (no. 1) requires a signed integer but the argument type is 'size_t {aka unsigned long long}'.\n"
                      "[test.cpp:9]: (warning) %Iu in format string (no. 2) requires an unsigned integer but the argument type is 'ptrdiff_t {aka long long}'.\n"
                      "[test.cpp:10]: (warning) %I32u in format string (no. 2) requires an unsigned integer but the argument type is '__int32 {aka int}'.\n"
                      "[test.cpp:11]: (warning) %I32d in format string (no. 1) requires a signed integer but the argument type is 'unsigned __int32 {aka unsigned int}'.\n"
                      "[test.cpp:12]: (warning) %I64u in format string (no. 2) requires an unsigned long long integer but the argument type is '__int64 {aka long long}'.\n"
                      "[test.cpp:13]: (warning) %I64d in format string (no. 1) requires a signed long long integer but the argument type is 'unsigned __int64 {aka unsigned long long}'.\n", errout.str());

        check("void foo() {\n"
              "    size_t s;\n"
              "    int i;\n"
              "    printf(\"%I\", s);\n"
              "    printf(\"%I6\", s);\n"
              "    printf(\"%I6x\", s);\n"
              "    printf(\"%I16\", s);\n"
              "    printf(\"%I16x\", s);\n"
              "    printf(\"%I32\", s);\n"
              "    printf(\"%I64\", s);\n"
              "    printf(\"%I%i\", s, i);\n"
              "    printf(\"%I6%i\", s, i);\n"
              "    printf(\"%I6x%i\", s, i);\n"
              "    printf(\"%I16%i\", s, i);\n"
              "    printf(\"%I16x%i\", s, i);\n"
              "    printf(\"%I32%i\", s, i);\n"
              "    printf(\"%I64%i\", s, i);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (warning) 'I' in format string (no. 1) is a length modifier and cannot be used without a conversion specifier.\n"
                      "[test.cpp:5]: (warning) 'I' in format string (no. 1) is a length modifier and cannot be used without a conversion specifier.\n"
                      "[test.cpp:6]: (warning) 'I' in format string (no. 1) is a length modifier and cannot be used without a conversion specifier.\n"
                      "[test.cpp:7]: (warning) 'I' in format string (no. 1) is a length modifier and cannot be used without a conversion specifier.\n"
                      "[test.cpp:8]: (warning) 'I' in format string (no. 1) is a length modifier and cannot be used without a conversion specifier.\n"
                      "[test.cpp:9]: (warning) 'I32' in format string (no. 1) is a length modifier and cannot be used without a conversion specifier.\n"
                      "[test.cpp:10]: (warning) 'I64' in format string (no. 1) is a length modifier and cannot be used without a conversion specifier.\n"
                      "[test.cpp:11]: (warning) 'I' in format string (no. 1) is a length modifier and cannot be used without a conversion specifier.\n"
                      "[test.cpp:12]: (warning) 'I' in format string (no. 1) is a length modifier and cannot be used without a conversion specifier.\n"
                      "[test.cpp:13]: (warning) 'I' in format string (no. 1) is a length modifier and cannot be used without a conversion specifier.\n"
                      "[test.cpp:14]: (warning) 'I' in format string (no. 1) is a length modifier and cannot be used without a conversion specifier.\n"
                      "[test.cpp:15]: (warning) 'I' in format string (no. 1) is a length modifier and cannot be used without a conversion specifier.\n"
                      "[test.cpp:16]: (warning) 'I32' in format string (no. 1) is a length modifier and cannot be used without a conversion specifier.\n"
                      "[test.cpp:17]: (warning) 'I64' in format string (no. 1) is a length modifier and cannot be used without a conversion specifier.\n", errout.str());
    }

    void testlibrarycfg() {
        const char code[] = "void f() {\n"
                            "    format(\"%s\");\n"
                            "}";

        // no error if configuration for 'format' is not provided
        check(code);
        ASSERT_EQUALS("", errout.str());

        // error if configuration for 'format' is provided
        Library lib;
        lib.argumentChecks["format"][1].formatstr = true;
        check(code, false, false, Settings::Unspecified, &lib);
        ASSERT_EQUALS("[test.cpp:2]: (error) format format string has 1 parameters but only 0 are given.\n", errout.str());
    }
};

REGISTER_TEST(TestIO)
