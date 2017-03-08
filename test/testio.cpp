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

#include "checkio.h"
#include "testsuite.h"
#include "tokenize.h"


class TestIO : public TestFixture {
public:
    TestIO() : TestFixture("TestIO") {
    }

private:
    Settings settings;

    void run() {
        LOAD_LIB_2(settings.library, "std.cfg");
        LOAD_LIB_2(settings.library, "windows.cfg");
        LOAD_LIB_2(settings.library, "qt.cfg");

        TEST_CASE(coutCerrMisusage);

        TEST_CASE(wrongMode_simple);
        TEST_CASE(wrongMode_complex);
        TEST_CASE(useClosedFile);
        TEST_CASE(fileIOwithoutPositioning);
        TEST_CASE(seekOnAppendedFile);
        TEST_CASE(fflushOnInputStream);

        TEST_CASE(testScanf1); // Scanf without field limiters
        TEST_CASE(testScanf2);
        TEST_CASE(testScanf3); // #3494
        TEST_CASE(testScanf4); // #ticket 2553

        TEST_CASE(testScanfArgument);
        TEST_CASE(testPrintfArgument);
        TEST_CASE(testPosixPrintfScanfParameterPosition);  // #4900

        TEST_CASE(testMicrosoftPrintfArgument); // ticket #4902
        TEST_CASE(testMicrosoftScanfArgument);
        TEST_CASE(testMicrosoftCStringFormatArguments); // ticket #4920
        TEST_CASE(testMicrosoftSecurePrintfArgument);
        TEST_CASE(testMicrosoftSecureScanfArgument);

        TEST_CASE(testQStringFormatArguments);

        TEST_CASE(testTernary); // ticket #6182
        TEST_CASE(testUnsignedConst); // ticket #6132

        TEST_CASE(testAstType); // #7014
        TEST_CASE(testPrintf0WithSuffix); // ticket #7069
    }

    void check(const char* code, bool inconclusive = false, bool portability = false, Settings::PlatformType platform = Settings::Unspecified) {
        // Clear the error buffer..
        errout.str("");

        settings.clearEnabled();
        settings.addEnabled("warning");
        settings.addEnabled("style");
        if (portability)
            settings.addEnabled("portability");
        settings.inconclusive = inconclusive;
        settings.platform(platform);

        // Tokenize..
        Tokenizer tokenizer(&settings, this);
        std::istringstream istr(code);
        tokenizer.tokenize(istr, "test.cpp");

        // Check..
        CheckIO checkIO(&tokenizer, &settings, this);
        checkIO.checkWrongPrintfScanfArguments();

        // Simplify token list..
        tokenizer.simplifyTokenList2();
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
            "  std::cout << (std::cout);\n"
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
            "    unknownObject << std::cout;\n"
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
              "    f = _wfopen(name, L\"r\");\n"
              "    fread(buffer, 5, 6, f);\n"
              "    rewind(f);\n"
              "    fwrite(buffer, 5, 6, f);\n"
              "}", false, false, Settings::Win32W);
        ASSERT_EQUALS("[test.cpp:5]: (error) Write operation on a file that was opened only for reading.\n", errout.str());

        check("void foo(FILE*& f) {\n"
              "    f = _tfopen(name, _T(\"r\"));\n"
              "    fread(buffer, 5, 6, f);\n"
              "    rewind(f);\n"
              "    fwrite(buffer, 5, 6, f);\n"
              "}", false, false, Settings::Win32A);
        ASSERT_EQUALS("[test.cpp:5]: (error) Write operation on a file that was opened only for reading.\n", errout.str());

        check("void foo(FILE*& f) {\n"
              "    f = _tfopen(name, _T(\"r\"));\n"
              "    fread(buffer, 5, 6, f);\n"
              "    rewind(f);\n"
              "    fwrite(buffer, 5, 6, f);\n"
              "}", false, false, Settings::Win32W);
        ASSERT_EQUALS("[test.cpp:5]: (error) Write operation on a file that was opened only for reading.\n", errout.str());

        check("void foo(FILE*& f) {\n"
              "    _wfopen_s(&f, name, L\"r\");\n"
              "    fread(buffer, 5, 6, f);\n"
              "    rewind(f);\n"
              "    fwrite(buffer, 5, 6, f);\n"
              "}", false, false, Settings::Win32W);
        ASSERT_EQUALS("[test.cpp:5]: (error) Write operation on a file that was opened only for reading.\n", errout.str());

        check("void foo(FILE*& f) {\n"
              "    _tfopen_s(&f, name, _T(\"r\"));\n"
              "    fread(buffer, 5, 6, f);\n"
              "    rewind(f);\n"
              "    fwrite(buffer, 5, 6, f);\n"
              "}", false, false, Settings::Win32A);
        ASSERT_EQUALS("[test.cpp:5]: (error) Write operation on a file that was opened only for reading.\n", errout.str());

        check("void foo(FILE*& f) {\n"
              "    _tfopen_s(&f, name, _T(\"r\"));\n"
              "    fread(buffer, 5, 6, f);\n"
              "    rewind(f);\n"
              "    fwrite(buffer, 5, 6, f);\n"
              "}", false, false, Settings::Win32W);
        ASSERT_EQUALS("[test.cpp:5]: (error) Write operation on a file that was opened only for reading.\n", errout.str());

        check("void foo(FILE*& f) {\n"
              "    f = fopen(name, \"r+\");\n"
              "    fwrite(buffer, 5, 6, f);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo(FILE*& f) {\n"
              "    f = _wfopen(name, L\"r+\");\n"
              "    fwrite(buffer, 5, 6, f);\n"
              "}", false, false, Settings::Win32W);
        ASSERT_EQUALS("", errout.str());

        check("void foo(FILE*& f) {\n"
              "    f = _tfopen(name, _T(\"r+\"));\n"
              "    fwrite(buffer, 5, 6, f);\n"
              "}", false, false, Settings::Win32A);
        ASSERT_EQUALS("", errout.str());

        check("void foo(FILE*& f) {\n"
              "    f = _tfopen(name, _T(\"r+\"));\n"
              "    fwrite(buffer, 5, 6, f);\n"
              "}", false, false, Settings::Win32W);
        ASSERT_EQUALS("", errout.str());

        check("void foo(FILE*& f) {\n"
              "    _wfopen_s(&f, name, L\"r+\");\n"
              "    fwrite(buffer, 5, 6, f);\n"
              "}", false, false, Settings::Win32W);
        ASSERT_EQUALS("", errout.str());

        check("void foo(FILE*& f) {\n"
              "    _tfopen_s(&f, name, _T(\"r+\"));\n"
              "    fwrite(buffer, 5, 6, f);\n"
              "}", false, false, Settings::Win32A);
        ASSERT_EQUALS("", errout.str());

        check("void foo(FILE*& f) {\n"
              "    _tfopen_s(&f, name, _T(\"r+\"));\n"
              "    fwrite(buffer, 5, 6, f);\n"
              "}", false, false, Settings::Win32W);
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
        ASSERT_EQUALS("[test.cpp:4]: (warning) Repositioning operation performed on a file opened in append mode has no effect.\n"
                      "[test.cpp:5]: (error) Read operation on a file that was opened only for writing.\n", errout.str());

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

        // freopen and tmpfile
        check("void foo(FILE*& f) {\n"
              "    f = freopen(name, \"r\", f);\n"
              "    fwrite(buffer, 5, 6, f);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Write operation on a file that was opened only for reading.\n", errout.str());

        check("void foo(FILE*& f) {\n"
              "    f = _wfreopen(name, L\"r\", f);\n"
              "    fwrite(buffer, 5, 6, f);\n"
              "}", false, false, Settings::Win32W);
        ASSERT_EQUALS("[test.cpp:3]: (error) Write operation on a file that was opened only for reading.\n", errout.str());

        check("void foo(FILE*& f) {\n"
              "    f = _tfreopen(name, _T(\"r\"), f);\n"
              "    fwrite(buffer, 5, 6, f);\n"
              "}", false, false, Settings::Win32A);
        ASSERT_EQUALS("[test.cpp:3]: (error) Write operation on a file that was opened only for reading.\n", errout.str());

        check("void foo(FILE*& f) {\n"
              "    f = _tfreopen(name, _T(\"r\"), f);\n"
              "    fwrite(buffer, 5, 6, f);\n"
              "}", false, false, Settings::Win32W);
        ASSERT_EQUALS("[test.cpp:3]: (error) Write operation on a file that was opened only for reading.\n", errout.str());

        check("void foo(FILE*& f) {\n"
              "    f = _wfreopen_s(&f, name, L\"r\", f);\n"
              "    fwrite(buffer, 5, 6, f);\n"
              "}", false, false, Settings::Win32W);
        ASSERT_EQUALS("[test.cpp:3]: (error) Write operation on a file that was opened only for reading.\n", errout.str());

        check("void foo(FILE*& f) {\n"
              "    f = _tfreopen_s(&f, name, _T(\"r\"), f);\n"
              "    fwrite(buffer, 5, 6, f);\n"
              "}", false, false, Settings::Win32A);
        ASSERT_EQUALS("[test.cpp:3]: (error) Write operation on a file that was opened only for reading.\n", errout.str());

        check("void foo(FILE*& f) {\n"
              "    f = _tfreopen_s(&f, name, _T(\"r\"), f);\n"
              "    fwrite(buffer, 5, 6, f);\n"
              "}", false, false, Settings::Win32W);
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
              "    ungetwc(L'a', f);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Used file that is not opened.\n"
                      "[test.cpp:4]: (error) Used file that is not opened.\n"
                      "[test.cpp:5]: (error) Used file that is not opened.\n"
                      "[test.cpp:6]: (error) Used file that is not opened.\n"
                      "[test.cpp:7]: (error) Used file that is not opened.\n", errout.str());

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

        check("void chdcd_parse_nero(FILE *infile) {\n"
              "    switch (mode) {\n"
              "        case 0x0300:\n"
              "            fclose(infile);\n"
              "            exit(0);\n"
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

        // #1473
        check("void foo() {\n"
              "    FILE *a = fopen(\"aa\", \"r\");\n"
              "    while (fclose(a)) {}\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (error) Used file that is not opened.\n", errout.str());

        // #6823
        check("void foo() {\n"
              "    FILE f[2];\n"
              "    f[0] = fopen(\"1\", \"w\");\n"
              "    f[1] = fopen(\"2\", \"w\");\n"
              "    fclose(f[0]);\n"
              "    fclose(f[1]);\n"
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

        // #6452 - member functions
        check("class FileStream {\n"
              "    void insert(const ByteVector &data, ulong start);\n"
              "    void seek(long offset, Position p);\n"
              "    FileStreamPrivate *d;\n"
              "};\n"
              "void FileStream::insert(const ByteVector &data, ulong start) {\n"
              "    int bytesRead = fread(aboutToOverwrite.data(), 1, bufferLength, d->file);\n"
              "    seek(writePosition);\n"
              "    fwrite(buffer.data(), sizeof(char), buffer.size(), d->file);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("class FileStream {\n"
              "    void insert(const ByteVector &data, ulong start);\n"
              "    FileStreamPrivate *d;\n"
              "};\n"
              "void FileStream::insert(const ByteVector &data, ulong start) {\n"
              "    int bytesRead = fread(aboutToOverwrite.data(), 1, bufferLength, d->file);\n"
              "    unknown(writePosition);\n"
              "    fwrite(buffer.data(), sizeof(char), buffer.size(), d->file);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("class FileStream {\n"
              "    void insert(const ByteVector &data, ulong start);\n"
              "    FileStreamPrivate *d;\n"
              "};\n"
              "void known(int);\n"
              "void FileStream::insert(const ByteVector &data, ulong start) {\n"
              "    int bytesRead = fread(aboutToOverwrite.data(), 1, bufferLength, d->file);\n"
              "    known(writePosition);\n"
              "    fwrite(buffer.data(), sizeof(char), buffer.size(), d->file);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:9]: (error) Read and write operations without a call to a positioning function (fseek, fsetpos or rewind) or fflush in between result in undefined behaviour.\n", errout.str());

        check("class FileStream {\n"
              "    void insert(const ByteVector &data, ulong start);\n"
              "    FileStreamPrivate *d;\n"
              "};\n"
              "void known(int);\n"
              "void FileStream::insert(const ByteVector &data, ulong start) {\n"
              "    int bytesRead = fread(X::data(), 1, bufferLength, d->file);\n"
              "    known(writePosition);\n"
              "    fwrite(X::data(), sizeof(char), buffer.size(), d->file);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:9]: (error) Read and write operations without a call to a positioning function (fseek, fsetpos or rewind) or fflush in between result in undefined behaviour.\n", errout.str());
    }

    void seekOnAppendedFile() {
        check("void foo() {\n"
              "    FILE* f = fopen(\"\", \"a+\");\n"
              "    fseek(f, 0, SEEK_SET);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo() {\n"
              "    FILE* f = fopen(\"\", \"w\");\n"
              "    fseek(f, 0, SEEK_SET);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo() {\n"
              "    FILE* f = fopen(\"\", \"a\");\n"
              "    fseek(f, 0, SEEK_SET);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) Repositioning operation performed on a file opened in append mode has no effect.\n", errout.str());

        check("void foo() {\n"
              "    FILE* f = fopen(\"\", \"a\");\n"
              "    fflush(f);\n"
              "}");
        ASSERT_EQUALS("", errout.str()); // #5578

        check("void foo() {\n"
              "    FILE* f = fopen(\"\", \"a\");\n"
              "    fclose(f);\n"
              "    f = fopen(\"\", \"r\");\n"
              "    fseek(f, 0, SEEK_SET);\n"
              "}");
        ASSERT_EQUALS("", errout.str()); // #6566
    }

    void fflushOnInputStream() {
        check("void foo()\n"
              "{\n"
              "    fflush(stdin);\n"
              "}", false, true);
        ASSERT_EQUALS("[test.cpp:3]: (portability) fflush() called on input stream 'stdin' may result in undefined behaviour on non-linux systems.\n", errout.str());

        check("void foo()\n"
              "{\n"
              "    fflush(stdout);\n"
              "}", false, true);
        ASSERT_EQUALS("", errout.str());

        check("void foo(FILE*& f) {\n"
              "    f = fopen(path, \"r\");\n"
              "    fflush(f);\n"
              "}", false, true);
        ASSERT_EQUALS("[test.cpp:3]: (portability) fflush() called on input stream 'f' may result in undefined behaviour on non-linux systems.\n", errout.str());

        check("void foo(FILE*& f) {\n"
              "    f = fopen(path, \"w\");\n"
              "    fflush(f);\n"
              "}", false, true);
        ASSERT_EQUALS("", errout.str());

        check("void foo(FILE*& f) {\n"
              "    fflush(f);\n"
              "}", false, true);
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
        ASSERT_EQUALS("[test.cpp:4]: (warning) fscanf() without field width limits can crash with huge input data.\n"
                      "[test.cpp:5]: (warning) scanf() without field width limits can crash with huge input data.\n"
                      "[test.cpp:6]: (warning) scanf() without field width limits can crash with huge input data.\n"
                      "[test.cpp:7]: (warning) sscanf() without field width limits can crash with huge input data.\n"
                      "[test.cpp:8]: (warning) scanf() without field width limits can crash with huge input data.\n", errout.str());
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
        ASSERT_EQUALS("[test.cpp:4]: (warning) scanf format string requires 0 parameters but 1 is given.\n", errout.str());
    }

    void testScanf3() { // ticket #3494
        check("void f() {\n"
              "  char str[8];\n"
              "  scanf(\"%7c\", str);\n"
              "  scanf(\"%8c\", str);\n"
              "  scanf(\"%9c\", str);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:5]: (error) Width 9 given in format string (no. 1) is larger than destination buffer 'str[8]', use %8c to prevent overflowing it.\n", errout.str());
    }

    void testScanf4() { // ticket #2553
        check("void f()\n"
              "{\n"
              "  char str [8];\n"
              "  scanf (\"%70s\",str);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:4]: (error) Width 70 given in format string (no. 1) is larger than destination buffer 'str[8]', use %7s to prevent overflowing it.\n", errout.str());
    }


#define TEST_SCANF_CODE(format, type)\
    "void f(){" type " x; scanf(\"" format "\", &x);}"

#define TEST_SCANF_ERR(format, formatStr, type)\
    "[test.cpp:1]: (warning) " format " in format string (no. 1) requires '" formatStr " *' but the argument type is '" type " *'.\n"

#define TEST_SCANF_ERR_AKA(format, formatStr, type, akaType)\
    "[test.cpp:1]: (portability) " format " in format string (no. 1) requires '" formatStr " *' but the argument type is '" type " * {aka " akaType " *}'.\n"


    void testScanfNoWarn(const char *filename, unsigned int linenr, const char* code) {
        check(code, true, false, Settings::Unix32);
        assertEquals(filename, linenr, "", errout.str());
        check(code, true, false, Settings::Unix64);
        assertEquals(filename, linenr, "", errout.str());
        check(code, true, false, Settings::Win32A);
        assertEquals(filename, linenr, "", errout.str());
        check(code, true, false, Settings::Win64);
        assertEquals(filename, linenr, "", errout.str());
    }

    void testScanfWarn(const char *filename, unsigned int linenr,
                       const char* code, const char* testScanfErrString) {
        check(code, true, false, Settings::Unix32);
        assertEquals(filename, linenr, testScanfErrString, errout.str());
        check(code, true, false, Settings::Unix64);
        assertEquals(filename, linenr, testScanfErrString, errout.str());
        check(code, true, false, Settings::Win32A);
        assertEquals(filename, linenr, testScanfErrString, errout.str());
        check(code, true, false, Settings::Win64);
        assertEquals(filename, linenr, testScanfErrString, errout.str());
    }

    void testScanfWarnAka(const char *filename, unsigned int linenr,
                          const char* code, const char* testScanfErrAkaString, const char* testScanfErrAkaWin64String) {
        check(code, true, true, Settings::Unix32);
        assertEquals(filename, linenr, testScanfErrAkaString, errout.str());
        check(code, true, true, Settings::Unix64);
        assertEquals(filename, linenr, testScanfErrAkaString, errout.str());
        check(code, true, true, Settings::Win32A);
        assertEquals(filename, linenr, testScanfErrAkaString, errout.str());
        check(code, true, true, Settings::Win64);
        assertEquals(filename, linenr, testScanfErrAkaWin64String, errout.str());
    }

    void testScanfWarnAkaWin64(const char *filename, unsigned int linenr,
                               const char* code, const char* testScanfErrAkaWin64String) {
        check(code, true, true, Settings::Unix32);
        assertEquals(filename, linenr, "", errout.str());
        check(code, true, true, Settings::Unix64);
        assertEquals(filename, linenr, "", errout.str());
        check(code, true, true, Settings::Win32A);
        assertEquals(filename, linenr, "", errout.str());
        check(code, true, true, Settings::Win64);
        assertEquals(filename, linenr, testScanfErrAkaWin64String, errout.str());
    }

    void testScanfWarnAkaWin32(const char *filename, unsigned int linenr,
                               const char* code, const char* testScanfErrAkaString) {
        check(code, true, true, Settings::Unix32);
        assertEquals(filename, linenr, testScanfErrAkaString, errout.str());
        check(code, true, true, Settings::Unix64);
        assertEquals(filename, linenr, testScanfErrAkaString, errout.str());
        check(code, true, true, Settings::Win32A);
        assertEquals(filename, linenr, testScanfErrAkaString, errout.str());
        check(code, true, true, Settings::Win64);
        assertEquals(filename, linenr, "", errout.str());
    }

#define TEST_SCANF_NOWARN(FORMAT, FORMATSTR, TYPE) \
    testScanfNoWarn(__FILE__, __LINE__, TEST_SCANF_CODE(FORMAT, TYPE))
#define TEST_SCANF_WARN(FORMAT, FORMATSTR, TYPE) \
    testScanfWarn(__FILE__, __LINE__, TEST_SCANF_CODE(FORMAT, TYPE), TEST_SCANF_ERR(FORMAT, FORMATSTR, TYPE))
#define TEST_SCANF_WARN_AKA(FORMAT, FORMATSTR, TYPE, AKATYPE, AKATYPE_WIN64) \
    testScanfWarnAka(__FILE__, __LINE__, TEST_SCANF_CODE(FORMAT, TYPE), TEST_SCANF_ERR_AKA(FORMAT, FORMATSTR, TYPE, AKATYPE), TEST_SCANF_ERR_AKA(FORMAT, FORMATSTR, TYPE, AKATYPE_WIN64))
#define TEST_SCANF_WARN_AKA_WIN64(FORMAT, FORMATSTR, TYPE, AKATYPE_WIN64) \
    testScanfWarnAkaWin64(__FILE__, __LINE__, TEST_SCANF_CODE(FORMAT, TYPE), TEST_SCANF_ERR_AKA(FORMAT, FORMATSTR, TYPE, AKATYPE_WIN64))
#define TEST_SCANF_WARN_AKA_WIN32(FORMAT, FORMATSTR, TYPE, AKATYPE_WIN32) \
    testScanfWarnAkaWin32(__FILE__, __LINE__, TEST_SCANF_CODE(FORMAT, TYPE), TEST_SCANF_ERR_AKA(FORMAT, FORMATSTR, TYPE, AKATYPE_WIN32))

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
        ASSERT_EQUALS("[test.cpp:2]: (warning) scanf format string requires 0 parameters but 1 is given.\n"
                      "[test.cpp:3]: (warning) scanf format string requires 1 parameter but 2 are given.\n"
                      "[test.cpp:4]: (warning) fscanf format string requires 1 parameter but 2 are given.\n"
                      "[test.cpp:5]: (warning) scanf format string requires 2 parameters but 3 are given.\n", errout.str());

        check("void foo() {\n"
              "    scanf(\"%1d\");\n"
              "    scanf(\"%1u%1u\", bar());\n"
              "    sscanf(bar, \"%1d%1d\", &foo);\n"
              "    scanf(\"%*1x %1x %29s\", &count);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (error) scanf format string requires 1 parameter but only 0 are given.\n"
                      "[test.cpp:3]: (error) scanf format string requires 2 parameters but only 1 is given.\n"
                      "[test.cpp:4]: (error) sscanf format string requires 2 parameters but only 1 is given.\n"
                      "[test.cpp:5]: (error) scanf format string requires 2 parameters but only 1 is given.\n", errout.str());

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
                      "[test.cpp:4]: (warning) sscanf() without field width limits can crash with huge input data.\n", errout.str());

        check("void foo() {\n"
              "    const size_t BUFLENGTH(2048);\n"
              "    typedef char bufT[BUFLENGTH];\n"
              "    bufT line= {0};\n"
              "    bufT projectId= {0};\n"
              "    const int scanrc=sscanf(line, \"Project(\\\"{%36s}\\\")\", projectId);\n"
              "    sscanf(input, \"%5s\", output);\n"
              "}", true);
        ASSERT_EQUALS("", errout.str());

        TEST_SCANF_WARN("%u", "unsigned int", "bool");
        TEST_SCANF_WARN("%u", "unsigned int", "char");
        TEST_SCANF_WARN("%u", "unsigned int", "signed char");
        TEST_SCANF_WARN("%u", "unsigned int", "unsigned char");
        TEST_SCANF_WARN("%u", "unsigned int", "signed short");
        TEST_SCANF_WARN("%u", "unsigned int", "unsigned short");
        TEST_SCANF_WARN("%u", "unsigned int", "signed int");
        TEST_SCANF_NOWARN("%u", "unsigned int", "unsigned int");
        TEST_SCANF_WARN("%u", "unsigned int", "signed long");
        TEST_SCANF_WARN("%u", "unsigned int", "unsigned long");
        TEST_SCANF_WARN("%u", "unsigned int", "signed long long");
        TEST_SCANF_WARN("%u", "unsigned int", "unsigned long long");
        TEST_SCANF_WARN("%u", "unsigned int", "float");
        TEST_SCANF_WARN("%u", "unsigned int", "double");
        TEST_SCANF_WARN("%u", "unsigned int", "long double");
        TEST_SCANF_WARN("%u", "unsigned int", "void *");
        TEST_SCANF_WARN_AKA("%u", "unsigned int", "size_t", "unsigned long", "unsigned long long");
        TEST_SCANF_WARN_AKA("%u", "unsigned int", "ssize_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%u", "unsigned int", "ptrdiff_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%u", "unsigned int", "intmax_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%u", "unsigned int", "uintmax_t", "unsigned long", "unsigned long long");
        TEST_SCANF_WARN_AKA("%u", "unsigned int", "std::size_t", "unsigned long", "unsigned long long");
        TEST_SCANF_WARN_AKA("%u", "unsigned int", "std::ssize_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%u", "unsigned int", "std::ptrdiff_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%u", "unsigned int", "std::intptr_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%u", "unsigned int", "std::uintptr_t", "unsigned long", "unsigned long long");

        TEST_SCANF_WARN("%lu","unsigned long","bool");
        TEST_SCANF_WARN("%lu","unsigned long","char");
        TEST_SCANF_WARN("%lu","unsigned long","signed char");
        TEST_SCANF_WARN("%lu","unsigned long","unsigned char");
        TEST_SCANF_WARN("%lu","unsigned long","signed short");
        TEST_SCANF_WARN("%lu","unsigned long","unsigned short");
        TEST_SCANF_WARN("%lu","unsigned long","signed int");
        TEST_SCANF_WARN("%lu","unsigned long","unsigned int");
        TEST_SCANF_WARN("%lu","unsigned long","signed long");
        TEST_SCANF_NOWARN("%lu","unsigned long","unsigned long");
        TEST_SCANF_WARN("%lu","unsigned long","signed long long");
        TEST_SCANF_WARN("%lu","unsigned long","unsigned long long");
        TEST_SCANF_WARN("%lu","unsigned long","float");
        TEST_SCANF_WARN("%lu","unsigned long","double");
        TEST_SCANF_WARN("%lu","unsigned long","long double");
        TEST_SCANF_WARN("%lu","unsigned long","void *");
        TEST_SCANF_WARN_AKA("%lu","unsigned long","size_t","unsigned long","unsigned long long");
        TEST_SCANF_WARN_AKA("%lu","unsigned long","ssize_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%lu","unsigned long","ptrdiff_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%lu","unsigned long","intmax_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%lu","unsigned long","uintmax_t","unsigned long", "unsigned long long");
        TEST_SCANF_WARN_AKA("%lu","unsigned long","std::size_t","unsigned long", "unsigned long long");
        TEST_SCANF_WARN_AKA("%lu","unsigned long","std::ssize_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%lu","unsigned long","std::ptrdiff_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%lu","unsigned long","std::intptr_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA_WIN64("%lu","unsigned long","std::uintptr_t", "unsigned long long");

        TEST_SCANF_WARN("%llu","unsigned long long","bool");
        TEST_SCANF_WARN("%llu","unsigned long long","char");
        TEST_SCANF_WARN("%llu","unsigned long long","signed char");
        TEST_SCANF_WARN("%llu","unsigned long long","unsigned char");
        TEST_SCANF_WARN("%llu","unsigned long long","signed short");
        TEST_SCANF_WARN("%llu","unsigned long long","unsigned short");
        TEST_SCANF_WARN("%llu","unsigned long long","signed int");
        TEST_SCANF_WARN("%llu","unsigned long long","unsigned int");
        TEST_SCANF_WARN("%llu","unsigned long long","signed long");
        TEST_SCANF_WARN("%llu","unsigned long long","unsigned long");
        TEST_SCANF_WARN("%llu","unsigned long long","signed long long");
        TEST_SCANF_NOWARN("%llu","unsigned long long","unsigned long long");
        TEST_SCANF_WARN("%llu","unsigned long long","float");
        TEST_SCANF_WARN("%llu","unsigned long long","double");
        TEST_SCANF_WARN("%llu","unsigned long long","long double");
        TEST_SCANF_WARN("%llu","unsigned long long","void *");
        TEST_SCANF_WARN_AKA("%llu","unsigned long long","size_t", "unsigned long", "unsigned long long");
        TEST_SCANF_WARN_AKA("%llu","unsigned long long","ssize_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%llu","unsigned long long","ptrdiff_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%llu","unsigned long long","intmax_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%llu","unsigned long long","uintmax_t", "unsigned long", "unsigned long long");
        TEST_SCANF_WARN_AKA("%llu","unsigned long long","std::size_t", "unsigned long", "unsigned long long");
        TEST_SCANF_WARN_AKA("%llu","unsigned long long","std::ssize_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%llu","unsigned long long","std::ptrdiff_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%llu","unsigned long long","std::intptr_t", "signed long", "signed long long");
        //TEST_SCANF_WARN_AKA("%llu","unsigned long long","std::uintptr_t", "unsigned long long");

        TEST_SCANF_WARN("%hu", "unsigned short", "bool");
        TEST_SCANF_WARN("%hu", "unsigned short", "char");
        TEST_SCANF_WARN("%hu", "unsigned short", "signed char");
        TEST_SCANF_WARN("%hu", "unsigned short", "unsigned char");
        TEST_SCANF_WARN("%hu", "unsigned short", "signed short");
        TEST_SCANF_NOWARN("%hu", "unsigned short", "unsigned short");
        TEST_SCANF_WARN("%hu", "unsigned short", "signed int");
        TEST_SCANF_WARN("%hu", "unsigned short", "unsigned int");
        TEST_SCANF_WARN("%hu", "unsigned short", "signed long");
        TEST_SCANF_WARN("%hu", "unsigned short", "unsigned long");
        TEST_SCANF_WARN("%hu", "unsigned short", "signed long long");
        TEST_SCANF_WARN("%hu", "unsigned short", "unsigned long long");
        TEST_SCANF_WARN("%hu", "unsigned short", "float");
        TEST_SCANF_WARN("%hu", "unsigned short", "double");
        TEST_SCANF_WARN("%hu", "unsigned short", "long double");
        TEST_SCANF_WARN("%hu", "unsigned short", "void *");
        TEST_SCANF_WARN_AKA("%hu", "unsigned short", "size_t", "unsigned long", "unsigned long long");
        TEST_SCANF_WARN_AKA("%hu", "unsigned short", "ssize_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%hu", "unsigned short", "ptrdiff_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%hu", "unsigned short", "intmax_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%hu", "unsigned short", "uintmax_t", "unsigned long", "unsigned long long");
        TEST_SCANF_WARN_AKA("%hu", "unsigned short", "std::size_t", "unsigned long", "unsigned long long");
        TEST_SCANF_WARN_AKA("%hu", "unsigned short", "std::ssize_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%hu", "unsigned short", "std::ptrdiff_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%hu", "unsigned short", "std::intptr_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%hu", "unsigned short", "std::uintptr_t", "unsigned long", "unsigned long long");

        TEST_SCANF_WARN("%hhu", "unsigned char", "bool");
        TEST_SCANF_WARN("%hhu", "unsigned char", "char");
        TEST_SCANF_WARN("%hhu", "unsigned char", "signed char");
        TEST_SCANF_NOWARN("%hhu", "unsigned char", "unsigned char");
        TEST_SCANF_WARN("%hhu", "unsigned char", "signed short");
        TEST_SCANF_WARN("%hhu", "unsigned char", "unsigned short");
        TEST_SCANF_WARN("%hhu", "unsigned char", "signed int");
        TEST_SCANF_WARN("%hhu", "unsigned char", "unsigned int");
        TEST_SCANF_WARN("%hhu", "unsigned char", "signed long");
        TEST_SCANF_WARN("%hhu", "unsigned char", "unsigned long");
        TEST_SCANF_WARN("%hhu", "unsigned char", "signed long long");
        TEST_SCANF_WARN("%hhu", "unsigned char", "unsigned long long");
        TEST_SCANF_WARN("%hhu", "unsigned char", "float");
        TEST_SCANF_WARN("%hhu", "unsigned char", "double");
        TEST_SCANF_WARN("%hhu", "unsigned char", "long double");
        TEST_SCANF_WARN("%hhu", "unsigned char", "void *");
        TEST_SCANF_WARN_AKA("%hhu", "unsigned char", "size_t", "unsigned long", "unsigned long long");
        TEST_SCANF_WARN_AKA("%hhu", "unsigned char", "ssize_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%hhu", "unsigned char", "ptrdiff_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%hhu", "unsigned char", "intmax_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%hhu", "unsigned char", "uintmax_t", "unsigned long", "unsigned long long");
        TEST_SCANF_WARN_AKA("%hhu", "unsigned char", "std::size_t", "unsigned long", "unsigned long long");
        TEST_SCANF_WARN_AKA("%hhu", "unsigned char", "std::ssize_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%hhu", "unsigned char", "std::ptrdiff_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%hhu", "unsigned char", "std::intptr_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%hhu", "unsigned char", "std::uintptr_t", "unsigned long", "unsigned long long");

        TEST_SCANF_WARN("%Lu", "unsigned long long", "bool");
        TEST_SCANF_WARN("%Lu", "unsigned long long", "char");
        TEST_SCANF_WARN("%Lu", "unsigned long long", "signed char");
        TEST_SCANF_WARN("%Lu", "unsigned long long", "unsigned char");
        TEST_SCANF_WARN("%Lu", "unsigned long long", "signed short");
        TEST_SCANF_WARN("%Lu", "unsigned long long", "unsigned short");
        TEST_SCANF_WARN("%Lu", "unsigned long long", "signed int");
        TEST_SCANF_WARN("%Lu", "unsigned long long", "unsigned int");
        TEST_SCANF_WARN("%Lu", "unsigned long long", "signed long");
        TEST_SCANF_WARN("%Lu", "unsigned long long", "unsigned long");
        TEST_SCANF_WARN("%Lu", "unsigned long long", "signed long long");
        TEST_SCANF_NOWARN("%Lu", "unsigned long long", "unsigned long long");
        TEST_SCANF_WARN("%Lu", "unsigned long long", "float");
        TEST_SCANF_WARN("%Lu", "unsigned long long", "double");
        TEST_SCANF_WARN("%Lu", "unsigned long long", "long double");
        TEST_SCANF_WARN("%Lu", "unsigned long long", "void *");
        TEST_SCANF_WARN_AKA("%Lu", "unsigned long long", "size_t", "unsigned long", "unsigned long long");
        TEST_SCANF_WARN_AKA("%Lu", "unsigned long long", "ssize_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%Lu", "unsigned long long", "ptrdiff_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%Lu", "unsigned long long", "intmax_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%Lu", "unsigned long long", "uintmax_t", "unsigned long", "unsigned long long");
        TEST_SCANF_WARN_AKA("%Lu", "unsigned long long", "std::size_t", "unsigned long", "unsigned long long");
        TEST_SCANF_WARN_AKA("%Lu", "unsigned long long", "std::ssize_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%Lu", "unsigned long long", "std::ptrdiff_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%Lu", "unsigned long long", "std::intptr_t", "signed long", "signed long long");
        // TODO TEST_SCANF_WARN_AKA("%Lu", "unsigned long long", "std::uintptr_t", "unsigned long", "unsigned long long");

        TEST_SCANF_WARN("%ju", "uintmax_t", "bool");
        TEST_SCANF_WARN("%ju", "uintmax_t", "char");
        TEST_SCANF_WARN("%ju", "uintmax_t", "signed char");
        TEST_SCANF_WARN("%ju", "uintmax_t", "unsigned char");
        TEST_SCANF_WARN("%ju", "uintmax_t", "signed short");
        TEST_SCANF_WARN("%ju", "uintmax_t", "unsigned short");
        TEST_SCANF_WARN("%ju", "uintmax_t", "signed int");
        TEST_SCANF_WARN("%ju", "uintmax_t", "unsigned int");
        TEST_SCANF_WARN("%ju", "uintmax_t", "signed long");
        TEST_SCANF_WARN("%ju", "uintmax_t", "unsigned long");
        TEST_SCANF_WARN("%ju", "uintmax_t", "signed long long");
        TEST_SCANF_WARN("%ju", "uintmax_t", "unsigned long long");
        TEST_SCANF_WARN("%ju", "uintmax_t", "float");
        TEST_SCANF_WARN("%ju", "uintmax_t", "double");
        TEST_SCANF_WARN("%ju", "uintmax_t", "long double");
        TEST_SCANF_WARN("%ju", "uintmax_t", "void *");
        TEST_SCANF_WARN_AKA("%ju", "uintmax_t", "size_t", "unsigned long", "unsigned long long");
        TEST_SCANF_WARN_AKA("%ju", "uintmax_t", "ssize_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%ju", "uintmax_t", "ptrdiff_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%ju", "uintmax_t", "intmax_t", "signed long", "signed long long");
        TEST_SCANF_NOWARN("%ju", "uintmax_t", "uintmax_t");
        TEST_SCANF_WARN_AKA("%ju", "uintmax_t", "std::size_t", "unsigned long", "unsigned long long");
        TEST_SCANF_WARN_AKA("%ju", "uintmax_t", "std::ssize_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%ju", "uintmax_t", "std::ptrdiff_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%ju", "uintmax_t", "std::intptr_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%ju", "uintmax_t", "std::uintptr_t", "unsigned long", "unsigned long long");

        TEST_SCANF_WARN("%zu", "size_t", "bool");
        TEST_SCANF_WARN("%zu", "size_t", "char");
        TEST_SCANF_WARN("%zu", "size_t", "signed char");
        TEST_SCANF_WARN("%zu", "size_t", "unsigned char");
        TEST_SCANF_WARN("%zu", "size_t", "signed short");
        TEST_SCANF_WARN("%zu", "size_t", "unsigned short");
        TEST_SCANF_WARN("%zu", "size_t", "signed int");
        TEST_SCANF_WARN("%zu", "size_t", "unsigned int");
        TEST_SCANF_WARN("%zu", "size_t", "signed long");
        TEST_SCANF_WARN("%zu", "size_t", "unsigned long");
        TEST_SCANF_WARN("%zu", "size_t", "signed long long");
        TEST_SCANF_WARN("%zu", "size_t", "unsigned long long");
        TEST_SCANF_WARN("%zu", "size_t", "float");
        TEST_SCANF_WARN("%zu", "size_t", "double");
        TEST_SCANF_WARN("%zu", "size_t", "long double");
        TEST_SCANF_WARN("%zu", "size_t", "void *");
        TEST_SCANF_NOWARN("%zu", "size_t", "size_t");
        TEST_SCANF_WARN_AKA("%zu", "size_t", "ssize_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%zu", "size_t", "ptrdiff_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%zu", "size_t", "intmax_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%zu", "size_t", "uintmax_t", "unsigned long", "unsigned long long");
        TEST_SCANF_NOWARN("%zu", "size_t", "std::size_t");
        TEST_SCANF_WARN_AKA("%zu", "size_t", "std::ssize_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%zu", "size_t", "std::ptrdiff_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%zu", "size_t", "std::intptr_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%zu", "size_t", "std::uintptr_t", "unsigned long", "unsigned long long");

        TEST_SCANF_WARN("%tu", "unsigned ptrdiff_t", "bool");
        TEST_SCANF_WARN("%tu", "unsigned ptrdiff_t", "char");
        TEST_SCANF_WARN("%tu", "unsigned ptrdiff_t", "signed char");
        TEST_SCANF_WARN("%tu", "unsigned ptrdiff_t", "unsigned char");
        TEST_SCANF_WARN("%tu", "unsigned ptrdiff_t", "signed short");
        TEST_SCANF_WARN("%tu", "unsigned ptrdiff_t", "unsigned short");
        TEST_SCANF_WARN("%tu", "unsigned ptrdiff_t", "signed int");
        TEST_SCANF_WARN("%tu", "unsigned ptrdiff_t", "unsigned int");
        TEST_SCANF_WARN("%tu", "unsigned ptrdiff_t", "signed long");
        TEST_SCANF_WARN("%tu", "unsigned ptrdiff_t", "unsigned long");
        TEST_SCANF_WARN("%tu", "unsigned ptrdiff_t", "signed long long");
        TEST_SCANF_WARN("%tu", "unsigned ptrdiff_t", "unsigned long long");
        TEST_SCANF_WARN("%tu", "unsigned ptrdiff_t", "float");
        TEST_SCANF_WARN("%tu", "unsigned ptrdiff_t", "double");
        TEST_SCANF_WARN("%tu", "unsigned ptrdiff_t", "long double");
        TEST_SCANF_WARN("%tu", "unsigned ptrdiff_t", "void *");
        TEST_SCANF_WARN_AKA("%tu", "unsigned ptrdiff_t", "size_t", "unsigned long", "unsigned long long");
        TEST_SCANF_WARN_AKA("%tu", "unsigned ptrdiff_t", "ssize_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%tu", "unsigned ptrdiff_t", "ptrdiff_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%tu", "unsigned ptrdiff_t", "intmax_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%tu", "unsigned ptrdiff_t", "uintmax_t", "unsigned long", "unsigned long long");
        TEST_SCANF_WARN_AKA("%tu", "unsigned ptrdiff_t", "std::size_t", "unsigned long", "unsigned long long");
        TEST_SCANF_WARN_AKA("%tu", "unsigned ptrdiff_t", "std::ssize_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%tu", "unsigned ptrdiff_t", "std::ptrdiff_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%tu", "unsigned ptrdiff_t", "std::intptr_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%tu", "unsigned ptrdiff_t", "std::uintptr_t", "unsigned long", "unsigned long long");

        TEST_SCANF_WARN("%I64u", "unsigned __int64", "bool");
        TEST_SCANF_WARN("%I64u", "unsigned __int64", "char");
        TEST_SCANF_WARN("%I64u", "unsigned __int64", "signed char");
        TEST_SCANF_WARN("%I64u", "unsigned __int64", "unsigned char");
        TEST_SCANF_WARN("%I64u", "unsigned __int64", "signed short");
        TEST_SCANF_WARN("%I64u", "unsigned __int64", "unsigned short");
        TEST_SCANF_WARN("%I64u", "unsigned __int64", "signed int");
        TEST_SCANF_WARN("%I64u", "unsigned __int64", "unsigned int");
        TEST_SCANF_WARN("%I64u", "unsigned __int64", "signed long");
        TEST_SCANF_WARN("%I64u", "unsigned __int64", "unsigned long");
        TEST_SCANF_WARN("%I64u", "unsigned __int64", "signed long long");
        TEST_SCANF_NOWARN("%I64u", "unsigned __int64", "unsigned long long");
        TEST_SCANF_WARN("%I64u", "unsigned __int64", "float");
        TEST_SCANF_WARN("%I64u", "unsigned __int64", "double");
        TEST_SCANF_WARN("%I64u", "unsigned __int64", "long double");
        TEST_SCANF_WARN_AKA_WIN32("%I64u", "unsigned __int64", "size_t", "unsigned long");
        TEST_SCANF_WARN_AKA("%I64u", "unsigned __int64", "ssize_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%I64u", "unsigned __int64", "ptrdiff_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%I64u", "unsigned __int64", "intmax_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA_WIN32("%I64u", "unsigned __int64", "uintmax_t", "unsigned long");
        TEST_SCANF_WARN_AKA_WIN32("%I64u", "unsigned __int64", "std::size_t", "unsigned long");
        TEST_SCANF_WARN_AKA("%I64u", "unsigned __int64", "std::ssize_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%I64u", "unsigned __int64", "std::ptrdiff_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%I64u", "unsigned __int64", "std::intptr_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA_WIN32("%I64u", "unsigned __int64", "std::uintptr_t", "unsigned long");

        TEST_SCANF_WARN("%d", "int", "bool");
        TEST_SCANF_WARN("%d", "int", "char");
        TEST_SCANF_WARN("%d", "int", "signed char");
        TEST_SCANF_WARN("%d", "int", "unsigned char");
        TEST_SCANF_WARN("%d", "int", "signed short");
        TEST_SCANF_WARN("%d", "int", "unsigned short");
        TEST_SCANF_NOWARN("%d", "int", "signed int");
        TEST_SCANF_WARN("%d", "int", "unsigned int");
        TEST_SCANF_WARN("%d", "int", "signed long");
        TEST_SCANF_WARN("%d", "int", "unsigned long");
        TEST_SCANF_WARN("%d", "int", "signed long long");
        TEST_SCANF_WARN("%d", "int", "unsigned long long");
        TEST_SCANF_WARN("%d", "int", "float");
        TEST_SCANF_WARN("%d", "int", "double");
        TEST_SCANF_WARN("%d", "int", "long double");
        TEST_SCANF_WARN("%d", "int", "void *");
        TEST_SCANF_WARN_AKA("%d", "int", "size_t", "unsigned long", "unsigned long long");
        TEST_SCANF_WARN_AKA("%d", "int", "ssize_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%d", "int", "ptrdiff_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%d", "int", "intmax_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%d", "int", "uintmax_t", "unsigned long", "unsigned long long");
        TEST_SCANF_WARN_AKA("%d", "int", "std::size_t", "unsigned long", "unsigned long long");
        TEST_SCANF_WARN_AKA("%d", "int", "std::ssize_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%d", "int", "std::ptrdiff_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%d", "int", "std::intptr_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%d", "int", "std::uintptr_t", "unsigned long", "unsigned long long");

        TEST_SCANF_WARN("%x", "unsigned int", "bool");
        TEST_SCANF_WARN("%x", "unsigned int", "char");
        TEST_SCANF_WARN("%x", "unsigned int", "signed char");
        TEST_SCANF_WARN("%x", "unsigned int", "unsigned char");
        TEST_SCANF_WARN("%x", "unsigned int", "signed short");
        TEST_SCANF_WARN("%x", "unsigned int", "unsigned short");
        TEST_SCANF_WARN("%x", "unsigned int", "signed int");
        TEST_SCANF_NOWARN("%x", "unsigned int", "unsigned int");
        TEST_SCANF_WARN("%x", "unsigned int", "signed long");
        TEST_SCANF_WARN("%x", "unsigned int", "unsigned long");
        TEST_SCANF_WARN("%x", "unsigned int", "signed long long");
        TEST_SCANF_WARN("%x", "unsigned int", "unsigned long long");
        TEST_SCANF_WARN("%x", "unsigned int", "float");
        TEST_SCANF_WARN("%x", "unsigned int", "double");
        TEST_SCANF_WARN("%x", "unsigned int", "long double");
        TEST_SCANF_WARN("%x", "unsigned int", "void *");
        TEST_SCANF_WARN_AKA("%x", "unsigned int", "size_t", "unsigned long", "unsigned long long");
        TEST_SCANF_WARN_AKA("%x", "unsigned int", "ssize_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%x", "unsigned int", "ptrdiff_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%x", "unsigned int", "intmax_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%x", "unsigned int", "uintmax_t", "unsigned long", "unsigned long long");
        TEST_SCANF_WARN_AKA("%x", "unsigned int", "std::size_t", "unsigned long", "unsigned long long");
        TEST_SCANF_WARN_AKA("%x", "unsigned int", "std::ssize_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%x", "unsigned int", "std::ptrdiff_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%x", "unsigned int", "std::intptr_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%x", "unsigned int", "std::uintptr_t", "unsigned long", "unsigned long long");

        TEST_SCANF_WARN("%f", "float", "bool");
        TEST_SCANF_WARN("%f", "float", "char");
        TEST_SCANF_WARN("%f", "float", "signed char");
        TEST_SCANF_WARN("%f", "float", "unsigned char");
        TEST_SCANF_WARN("%f", "float", "signed short");
        TEST_SCANF_WARN("%f", "float", "unsigned short");
        TEST_SCANF_WARN("%f", "float", "signed int");
        TEST_SCANF_WARN("%f", "float", "unsigned int");
        TEST_SCANF_WARN("%f", "float", "signed long");
        TEST_SCANF_WARN("%f", "float", "unsigned long");
        TEST_SCANF_WARN("%f", "float", "signed long long");
        TEST_SCANF_WARN("%f", "float", "unsigned long long");
        TEST_SCANF_NOWARN("%f", "float", "float");
        TEST_SCANF_WARN("%f", "float", "double");
        TEST_SCANF_WARN("%f", "float", "long double");
        TEST_SCANF_WARN("%f", "float", "void *");
        TEST_SCANF_WARN_AKA("%f", "float", "size_t", "unsigned long", "unsigned long long");
        TEST_SCANF_WARN_AKA("%f", "float", "ssize_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%f", "float", "ptrdiff_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%f", "float", "intmax_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%f", "float", "uintmax_t", "unsigned long", "unsigned long long");
        TEST_SCANF_WARN_AKA("%f", "float", "std::size_t", "unsigned long", "unsigned long long");
        TEST_SCANF_WARN_AKA("%f", "float", "std::ssize_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%f", "float", "std::ptrdiff_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%f", "float", "std::intptr_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%f", "float", "std::uintptr_t", "unsigned long", "unsigned long long");

        TEST_SCANF_WARN("%lf", "double", "bool");
        TEST_SCANF_WARN("%lf", "double", "char");
        TEST_SCANF_WARN("%lf", "double", "signed char");
        TEST_SCANF_WARN("%lf", "double", "unsigned char");
        TEST_SCANF_WARN("%lf", "double", "signed short");
        TEST_SCANF_WARN("%lf", "double", "unsigned short");
        TEST_SCANF_WARN("%lf", "double", "signed int");
        TEST_SCANF_WARN("%lf", "double", "unsigned int");
        TEST_SCANF_WARN("%lf", "double", "signed long");
        TEST_SCANF_WARN("%lf", "double", "unsigned long");
        TEST_SCANF_WARN("%lf", "double", "signed long long");
        TEST_SCANF_WARN("%lf", "double", "unsigned long long");
        TEST_SCANF_WARN("%lf", "double", "float");
        TEST_SCANF_NOWARN("%lf", "double", "double");
        TEST_SCANF_WARN("%lf", "double", "long double");
        TEST_SCANF_WARN("%lf", "double", "void *");
        TEST_SCANF_WARN_AKA("%lf", "double", "size_t", "unsigned long", "unsigned long long");
        TEST_SCANF_WARN_AKA("%lf", "double", "ssize_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%lf", "double", "ptrdiff_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%lf", "double", "intmax_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%lf", "double", "uintmax_t", "unsigned long", "unsigned long long");
        TEST_SCANF_WARN_AKA("%lf", "double", "std::size_t", "unsigned long", "unsigned long long");
        TEST_SCANF_WARN_AKA("%lf", "double", "std::ssize_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%lf", "double", "std::ptrdiff_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%lf", "double", "std::intptr_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%lf", "double", "std::uintptr_t", "unsigned long", "unsigned long long");

        TEST_SCANF_WARN("%Lf", "long double", "bool");
        TEST_SCANF_WARN("%Lf", "long double", "char");
        TEST_SCANF_WARN("%Lf", "long double", "signed char");
        TEST_SCANF_WARN("%Lf", "long double", "unsigned char");
        TEST_SCANF_WARN("%Lf", "long double", "signed short");
        TEST_SCANF_WARN("%Lf", "long double", "unsigned short");
        TEST_SCANF_WARN("%Lf", "long double", "signed int");
        TEST_SCANF_WARN("%Lf", "long double", "unsigned int");
        TEST_SCANF_WARN("%Lf", "long double", "signed long");
        TEST_SCANF_WARN("%Lf", "long double", "unsigned long");
        TEST_SCANF_WARN("%Lf", "long double", "signed long long");
        TEST_SCANF_WARN("%Lf", "long double", "unsigned long long");
        TEST_SCANF_WARN("%Lf", "long double", "float");
        TEST_SCANF_WARN("%Lf", "long double", "double");
        TEST_SCANF_NOWARN("%Lf", "long double", "long double");
        TEST_SCANF_WARN("%Lf", "long double", "void *");
        TEST_SCANF_WARN_AKA("%Lf", "long double", "size_t", "unsigned long", "unsigned long long");
        TEST_SCANF_WARN_AKA("%Lf", "long double", "ssize_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%Lf", "long double", "ptrdiff_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%Lf", "long double", "intmax_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%Lf", "long double", "uintmax_t", "unsigned long", "unsigned long long");
        TEST_SCANF_WARN_AKA("%Lf", "long double", "std::size_t", "unsigned long", "unsigned long long");
        TEST_SCANF_WARN_AKA("%Lf", "long double", "std::ssize_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%Lf", "long double", "std::ptrdiff_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%Lf", "long double", "std::intptr_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%Lf", "long double", "std::uintptr_t", "unsigned long", "unsigned long long");

        TEST_SCANF_WARN("%n", "int", "bool");
        TEST_SCANF_WARN("%n", "int", "char");
        TEST_SCANF_WARN("%n", "int", "signed char");
        TEST_SCANF_WARN("%n", "int", "unsigned char");
        TEST_SCANF_WARN("%n", "int", "signed short");
        TEST_SCANF_WARN("%n", "int", "unsigned short");
        TEST_SCANF_NOWARN("%n", "int", "signed int");
        TEST_SCANF_WARN("%n", "int", "unsigned int");
        TEST_SCANF_WARN("%n", "int", "signed long");
        TEST_SCANF_WARN("%n", "int", "unsigned long");
        TEST_SCANF_WARN("%n", "int", "signed long long");
        TEST_SCANF_WARN("%n", "int", "unsigned long long");
        TEST_SCANF_WARN("%n", "int", "float");
        TEST_SCANF_WARN("%n", "int", "double");
        TEST_SCANF_WARN("%n", "int", "long double");
        TEST_SCANF_WARN("%n", "int", "void *");
        TEST_SCANF_WARN_AKA("%n", "int", "size_t", "unsigned long", "unsigned long long");
        TEST_SCANF_WARN_AKA("%n", "int", "ssize_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%n", "int", "ptrdiff_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%n", "int", "intmax_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%n", "int", "uintmax_t", "unsigned long", "unsigned long long");
        TEST_SCANF_WARN_AKA("%n", "int", "std::size_t", "unsigned long", "unsigned long long");
        TEST_SCANF_WARN_AKA("%n", "int", "std::ssize_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%n", "int", "std::ptrdiff_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%n", "int", "std::intptr_t", "signed long", "signed long long");
        TEST_SCANF_WARN_AKA("%n", "int", "std::uintptr_t", "unsigned long", "unsigned long long");

        check("void g() {\n" // #5104
              "    myvector<int> v1(1);\n"
              "    scanf(\"%d\n\",&v1[0]);\n"
              "    myvector<unsigned int> v2(1);\n"
              "    scanf(\"%u\n\",&v2[0]);\n"
              "    myvector<unsigned int> v3(1);\n"
              "    scanf(\"%x\n\",&v3[0]);\n"
              "    myvector<double> v4(1);\n"
              "    scanf(\"%lf\n\",&v4[0]);\n"
              "    myvector<char *> v5(1);\n"
              "    scanf(\"%10s\n\",v5[0]);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        {
            const char * code = "void g() {\n" // #5348
                                "    size_t s1;\n"
                                "    ptrdiff_t s2;\n"
                                "    ssize_t s3;\n"
                                "    scanf(\"%zd\", &s1);\n"
                                "    scanf(\"%zd\", &s2);\n"
                                "    scanf(\"%zd\", &s3);\n"
                                "}\n";
            const char* result("[test.cpp:5]: (portability) %zd in format string (no. 1) requires 'ptrdiff_t *' but the argument type is 'size_t * {aka unsigned long *}'.\n");
            const char* result_win64("[test.cpp:5]: (portability) %zd in format string (no. 1) requires 'ptrdiff_t *' but the argument type is 'size_t * {aka unsigned long long *}'.\n");

            check(code, false, true, Settings::Unix32);
            ASSERT_EQUALS(result, errout.str());
            check(code, false, true, Settings::Unix64);
            ASSERT_EQUALS(result, errout.str());
            check(code, false, true, Settings::Win32A);
            ASSERT_EQUALS(result, errout.str());
            check(code, false, true, Settings::Win64);
            ASSERT_EQUALS(result_win64, errout.str());
        }
        {
            check("void g() {\n"
                  "    const char c[]=\"42\";\n"
                  "    scanf(\"%s\n\", c);\n"
                  "}\n");
            ASSERT_EQUALS("[test.cpp:3]: (warning) %s in format string (no. 1) requires a 'char *' but the argument type is 'const char *'.\n"
                          "[test.cpp:3]: (warning) scanf() without field width limits can crash with huge input data.\n", errout.str());
        }

    }

    void testPrintfArgument() {
        check("void foo() {\n"
              "    printf(\"%i\");\n"
              "    printf(\"%i%s\", 123);\n"
              "    printf(\"%i%s%d\", 0, bar());\n"
              "    printf(\"%i%%%s%d\", 0, bar());\n"
              "    printf(\"%idfd%%dfa%s%d\", 0, bar());\n"
              "    fprintf(stderr,\"%u%s\");\n"
              "    snprintf(str,10,\"%u%s\");\n"
              "    sprintf(string1, \"%-*.*s\", 32, string2);\n" // #3364
              "    snprintf(a, 9, \"%s%d\", \"11223344\");\n" // #3655
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (error) printf format string requires 1 parameter but only 0 are given.\n"
                      "[test.cpp:3]: (error) printf format string requires 2 parameters but only 1 is given.\n"
                      "[test.cpp:4]: (error) printf format string requires 3 parameters but only 2 are given.\n"
                      "[test.cpp:5]: (error) printf format string requires 3 parameters but only 2 are given.\n"
                      "[test.cpp:6]: (error) printf format string requires 3 parameters but only 2 are given.\n"
                      "[test.cpp:7]: (error) fprintf format string requires 2 parameters but only 0 are given.\n"
                      "[test.cpp:8]: (error) snprintf format string requires 2 parameters but only 0 are given.\n"
                      "[test.cpp:9]: (error) sprintf format string requires 3 parameters but only 2 are given.\n"
                      "[test.cpp:10]: (error) snprintf format string requires 2 parameters but only 1 is given.\n", errout.str());

        check("void foo(char *str) {\n"
              "    printf(\"\", 0);\n"
              "    printf(\"%i\", 123, bar());\n"
              "    printf(\"%i%s\", 0, bar(), 43123);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) printf format string requires 0 parameters but 1 is given.\n"
                      "[test.cpp:3]: (warning) printf format string requires 1 parameter but 2 are given.\n"
                      "[test.cpp:4]: (warning) printf format string requires 2 parameters but 3 are given.\n", errout.str());

        check("void foo() {\n" // swprintf exists as MSVC extension and as standard function: #4790
              "    swprintf(string1, L\"%i\", 32, string2);\n" // MSVC implementation
              "    swprintf(string1, L\"%s%s\", L\"a\", string2);\n" // MSVC implementation
              "    swprintf(string1, 6, L\"%i\", 32, string2);\n" // Standard implementation
              "    swprintf(string1, 6, L\"%i%s\", 32, string2);\n" // Standard implementation
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) swprintf format string requires 1 parameter but 2 are given.\n"
                      "[test.cpp:4]: (warning) swprintf format string requires 1 parameter but 2 are given.\n", errout.str());

        check("void foo(char *str) {\n"
              "    printf(\"%i\", 0);\n"
              "    printf(\"%i%s\", 123, bar());\n"
              "    printf(\"%i%s%d\", 0, bar(), 43123);\n"
              "    printf(\"%i%%%s%d\", 0, bar(), 43123);\n"
              "    printf(\"%idfd%%dfa%s%d\", 0, bar(), 43123);\n"
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
        ASSERT_EQUALS("[test.cpp:3]: (warning) %s in format string (no. 1) requires 'char *' but the argument type is 'signed int'.\n"
                      "[test.cpp:4]: (warning) %s in format string (no. 2) requires 'char *' but the argument type is 'signed int'.\n"
                      "[test.cpp:5]: (warning) %s in format string (no. 1) requires 'char *' but the argument type is 'std::string'.\n"
                      "[test.cpp:7]: (warning) %u in format string (no. 1) requires 'unsigned int' but the argument type is 'char *'.\n", errout.str());

        check("void foo(const int* cpi, const int ci, int i, int* pi, std::string s) {\n"
              "    printf(\"%n\", cpi);\n"
              "    printf(\"%n\", ci);\n"
              "    printf(\"%n\", i);\n"
              "    printf(\"%n\", pi);\n"
              "    printf(\"%n\", s);\n"
              "    printf(\"%n\", \"s4\");\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) %n in format string (no. 1) requires 'int *' but the argument type is 'signed int'.\n"
                      "[test.cpp:4]: (warning) %n in format string (no. 1) requires 'int *' but the argument type is 'signed int'.\n"
                      "[test.cpp:6]: (warning) %n in format string (no. 1) requires 'int *' but the argument type is 'std::string'.\n"
                      "[test.cpp:7]: (warning) %n in format string (no. 1) requires 'int *' but the argument type is 'const char *'.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:3]: (warning) %X in format string (no. 1) requires 'unsigned int' but the argument type is 'foo'.\n"
                      "[test.cpp:4]: (warning) %c in format string (no. 1) requires 'unsigned int' but the argument type is 'const char *'.\n"
                      "[test.cpp:5]: (warning) %o in format string (no. 1) requires 'unsigned int' but the argument type is 'double'.\n"
                      "[test.cpp:6]: (warning) %x in format string (no. 1) requires 'unsigned int' but the argument type is 'const signed int *'.\n"
                      "[test.cpp:8]: (warning) %X in format string (no. 1) requires 'unsigned int' but the argument type is 'bar *'.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:3]: (warning) %i in format string (no. 1) requires 'int' but the argument type is 'foo'.\n"
                      "[test.cpp:4]: (warning) %d in format string (no. 1) requires 'int' but the argument type is 'const char *'.\n"
                      "[test.cpp:5]: (warning) %d in format string (no. 1) requires 'int' but the argument type is 'double'.\n"
                      "[test.cpp:6]: (warning) %d in format string (no. 1) requires 'int' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:7]: (warning) %d in format string (no. 1) requires 'int' but the argument type is 'const signed int *'.\n"
                      "[test.cpp:9]: (warning) %i in format string (no. 1) requires 'int' but the argument type is 'bar *'.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:3]: (warning) %u in format string (no. 1) requires 'unsigned int' but the argument type is 'foo'.\n"
                      "[test.cpp:4]: (warning) %u in format string (no. 1) requires 'unsigned int' but the argument type is 'const char *'.\n"
                      "[test.cpp:5]: (warning) %u in format string (no. 1) requires 'unsigned int' but the argument type is 'double'.\n"
                      "[test.cpp:6]: (warning) %u in format string (no. 1) requires 'unsigned int' but the argument type is 'signed int'.\n"
                      "[test.cpp:7]: (warning) %u in format string (no. 1) requires 'unsigned int' but the argument type is 'const signed int *'.\n"
                      "[test.cpp:9]: (warning) %u in format string (no. 1) requires 'unsigned int' but the argument type is 'bar *'.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:3]: (warning) %e in format string (no. 1) requires 'double' but the argument type is 'foo'.\n"
                      "[test.cpp:4]: (warning) %E in format string (no. 1) requires 'double' but the argument type is 'const char *'.\n"
                      "[test.cpp:5]: (warning) %f in format string (no. 1) requires 'double' but the argument type is 'const signed int *'.\n"
                      "[test.cpp:6]: (warning) %G in format string (no. 1) requires 'double' but the argument type is 'bar *'.\n", errout.str());

        check("class foo;\n"
              "void foo(foo f) {\n"
              "    printf(\"%u\", f);\n"
              "    printf(\"%f\", f);\n"
              "    printf(\"%p\", f);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:3]: (warning) %u in format string (no. 1) requires 'unsigned int' but the argument type is 'foo'.\n"
                      "[test.cpp:4]: (warning) %f in format string (no. 1) requires 'double' but the argument type is 'foo'.\n"
                      "[test.cpp:5]: (warning) %p in format string (no. 1) requires an address but the argument type is 'foo'.\n", errout.str());

        // Ticket #4189 (Improve check (printf("%l") not detected)) tests (according to C99 7.19.6.1.7)
        // False positive tests
        check("void foo(signed char sc, unsigned char uc, short int si, unsigned short int usi) {\n"
              "  printf(\"%hhx %hhd\", sc, uc);\n"
              "  printf(\"%hd %hu\", si, usi);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) %hhd in format string (no. 2) requires 'char' but the argument type is 'unsigned char'.\n", errout.str());

        check("void foo(long long int lli, unsigned long long int ulli, long int li, unsigned long int uli) {\n"
              "  printf(\"%llo %llx\", lli, ulli);\n"
              "  printf(\"%ld %lu\", li, uli);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        check("void foo(intmax_t im, uintmax_t uim, size_t s, ptrdiff_t p, long double ld, std::size_t ss, std::ptrdiff_t sp) {\n"
              "  printf(\"%jd %jo\", im, uim);\n"
              "  printf(\"%zx\", s);\n"
              "  printf(\"%ti\", p);\n"
              "  printf(\"%La\", ld);\n"
              "  printf(\"%zx\", ss);\n"
              "  printf(\"%ti\", sp);\n"
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
        ASSERT_EQUALS("[test.cpp:2]: (warning) %hd in format string (no. 1) requires 'short' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:3]: (warning) %hhd in format string (no. 1) requires 'char' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:4]: (warning) %ld in format string (no. 1) requires 'long' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:5]: (warning) %lld in format string (no. 1) requires 'long long' but the argument type is 'unsigned int'.\n" , errout.str());

        check("void foo(size_t s, ptrdiff_t p) {\n"
              "  printf(\"%zd\", s);\n"
              "  printf(\"%tu\", p);\n"
              "}", false, true, Settings::Unix32);
        ASSERT_EQUALS("[test.cpp:2]: (portability) %zd in format string (no. 1) requires 'ssize_t' but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:3]: (portability) %tu in format string (no. 1) requires 'unsigned ptrdiff_t' but the argument type is 'ptrdiff_t {aka signed long}'.\n", errout.str());

        check("void foo(std::size_t s, std::ptrdiff_t p) {\n"
              "  printf(\"%zd\", s);\n"
              "  printf(\"%tu\", p);\n"
              "}", false, true, Settings::Unix32);
        ASSERT_EQUALS("[test.cpp:2]: (portability) %zd in format string (no. 1) requires 'ssize_t' but the argument type is 'std::size_t {aka unsigned long}'.\n"
                      "[test.cpp:3]: (portability) %tu in format string (no. 1) requires 'unsigned ptrdiff_t' but the argument type is 'std::ptrdiff_t {aka signed long}'.\n", errout.str());

        check("void foo(size_t s, ptrdiff_t p) {\n"
              "  printf(\"%zd\", s);\n"
              "  printf(\"%tu\", p);\n"
              "}", false, true, Settings::Unix64);
        ASSERT_EQUALS("[test.cpp:2]: (portability) %zd in format string (no. 1) requires 'ssize_t' but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:3]: (portability) %tu in format string (no. 1) requires 'unsigned ptrdiff_t' but the argument type is 'ptrdiff_t {aka signed long}'.\n", errout.str());

        check("void foo(std::size_t s, std::ptrdiff_t p) {\n"
              "  printf(\"%zd\", s);\n"
              "  printf(\"%tu\", p);\n"
              "}", false, true, Settings::Unix64);
        ASSERT_EQUALS("[test.cpp:2]: (portability) %zd in format string (no. 1) requires 'ssize_t' but the argument type is 'std::size_t {aka unsigned long}'.\n"
                      "[test.cpp:3]: (portability) %tu in format string (no. 1) requires 'unsigned ptrdiff_t' but the argument type is 'std::ptrdiff_t {aka signed long}'.\n", errout.str());

        check("void foo(size_t s, ptrdiff_t p) {\n"
              "  printf(\"%zd\", s);\n"
              "  printf(\"%tu\", p);\n"
              "}", false, true, Settings::Win32A);
        ASSERT_EQUALS("[test.cpp:2]: (portability) %zd in format string (no. 1) requires 'ssize_t' but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:3]: (portability) %tu in format string (no. 1) requires 'unsigned ptrdiff_t' but the argument type is 'ptrdiff_t {aka signed long}'.\n", errout.str());

        check("void foo(std::size_t s, std::ptrdiff_t p) {\n"
              "  printf(\"%zd\", s);\n"
              "  printf(\"%tu\", p);\n"
              "}", false, true, Settings::Win32A);
        ASSERT_EQUALS("[test.cpp:2]: (portability) %zd in format string (no. 1) requires 'ssize_t' but the argument type is 'std::size_t {aka unsigned long}'.\n"
                      "[test.cpp:3]: (portability) %tu in format string (no. 1) requires 'unsigned ptrdiff_t' but the argument type is 'std::ptrdiff_t {aka signed long}'.\n", errout.str());

        check("void foo(size_t s, ptrdiff_t p) {\n"
              "  printf(\"%zd\", s);\n"
              "  printf(\"%tu\", p);\n"
              "}", false, true, Settings::Win64);
        ASSERT_EQUALS("[test.cpp:2]: (portability) %zd in format string (no. 1) requires 'ssize_t' but the argument type is 'size_t {aka unsigned long long}'.\n"
                      "[test.cpp:3]: (portability) %tu in format string (no. 1) requires 'unsigned ptrdiff_t' but the argument type is 'ptrdiff_t {aka signed long long}'.\n", errout.str());

        check("void foo(std::size_t s, std::ptrdiff_t p) {\n"
              "  printf(\"%zd\", s);\n"
              "  printf(\"%tu\", p);\n"
              "}", false, true, Settings::Win64);
        ASSERT_EQUALS("[test.cpp:2]: (portability) %zd in format string (no. 1) requires 'ssize_t' but the argument type is 'std::size_t {aka unsigned long long}'.\n"
                      "[test.cpp:3]: (portability) %tu in format string (no. 1) requires 'unsigned ptrdiff_t' but the argument type is 'std::ptrdiff_t {aka signed long long}'.\n", errout.str());

        check("void foo(unsigned int i) {\n"
              "  printf(\"%ld\", i);\n"
              "  printf(\"%lld\", i);\n"
              "  printf(\"%lu\", i);\n"
              "  printf(\"%llu\", i);\n"
              "  printf(\"%lx\", i);\n"
              "  printf(\"%llx\", i);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) %ld in format string (no. 1) requires 'long' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:3]: (warning) %lld in format string (no. 1) requires 'long long' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:4]: (warning) %lu in format string (no. 1) requires 'unsigned long' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:5]: (warning) %llu in format string (no. 1) requires 'unsigned long long' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:6]: (warning) %lx in format string (no. 1) requires 'unsigned long' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:7]: (warning) %llx in format string (no. 1) requires 'unsigned long long' but the argument type is 'unsigned int'.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:13]: (warning) %d in format string (no. 1) requires 'int' but the argument type is 'double'.\n"
                      "[test.cpp:13]: (warning) %f in format string (no. 2) requires 'double' but the argument type is 'signed int'.\n"
                      "[test.cpp:13]: (warning) %f in format string (no. 3) requires 'double' but the argument type is 'int'.\n"
                      "[test.cpp:13]: (warning) %d in format string (no. 4) requires 'int' but the argument type is 'double'.\n"
                      "[test.cpp:13]: (warning) %f in format string (no. 5) requires 'double' but the argument type is 'int'.\n"
                      "[test.cpp:13]: (warning) %f in format string (no. 6) requires 'double' but the argument type is 'int'.\n", errout.str());

        check("short f() { return 0; }\n"
              "void foo() { printf(\"%d %u %lu %I64u %I64d %f %Lf %p\", f(), f(), f(), f(), f(), f(), f(), f()); }");
        ASSERT_EQUALS("[test.cpp:2]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'signed short'.\n"
                      "[test.cpp:2]: (warning) %lu in format string (no. 3) requires 'unsigned long' but the argument type is 'signed short'.\n"
                      "[test.cpp:2]: (warning) %I64u in format string (no. 4) requires 'unsigned __int64' but the argument type is 'signed short'.\n"
                      "[test.cpp:2]: (warning) %I64d in format string (no. 5) requires '__int64' but the argument type is 'signed short'.\n"
                      "[test.cpp:2]: (warning) %f in format string (no. 6) requires 'double' but the argument type is 'signed short'.\n"
                      "[test.cpp:2]: (warning) %Lf in format string (no. 7) requires 'long double' but the argument type is 'signed short'.\n"
                      "[test.cpp:2]: (warning) %p in format string (no. 8) requires an address but the argument type is 'signed short'.\n", errout.str());

        check("unsigned short f() { return 0; }\n"
              "void foo() { printf(\"%u %d %ld %I64d %I64u %f %Lf %p\", f(), f(), f(), f(), f(), f(), f(), f()); }");
        ASSERT_EQUALS("[test.cpp:2]: (warning) %ld in format string (no. 3) requires 'long' but the argument type is 'unsigned short'.\n"
                      "[test.cpp:2]: (warning) %I64d in format string (no. 4) requires '__int64' but the argument type is 'unsigned short'.\n"
                      "[test.cpp:2]: (warning) %I64u in format string (no. 5) requires 'unsigned __int64' but the argument type is 'unsigned short'.\n"
                      "[test.cpp:2]: (warning) %f in format string (no. 6) requires 'double' but the argument type is 'unsigned short'.\n"
                      "[test.cpp:2]: (warning) %Lf in format string (no. 7) requires 'long double' but the argument type is 'unsigned short'.\n"
                      "[test.cpp:2]: (warning) %p in format string (no. 8) requires an address but the argument type is 'unsigned short'.\n", errout.str());

        check("int f() { return 0; }\n"
              "void foo() { printf(\"%d %u %lu %I64u %I64d %f %Lf %p\", f(), f(), f(), f(), f(), f(), f(), f()); }");
        ASSERT_EQUALS("[test.cpp:2]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'signed int'.\n"
                      "[test.cpp:2]: (warning) %lu in format string (no. 3) requires 'unsigned long' but the argument type is 'signed int'.\n"
                      "[test.cpp:2]: (warning) %I64u in format string (no. 4) requires 'unsigned __int64' but the argument type is 'signed int'.\n"
                      "[test.cpp:2]: (warning) %I64d in format string (no. 5) requires '__int64' but the argument type is 'signed int'.\n"
                      "[test.cpp:2]: (warning) %f in format string (no. 6) requires 'double' but the argument type is 'signed int'.\n"
                      "[test.cpp:2]: (warning) %Lf in format string (no. 7) requires 'long double' but the argument type is 'signed int'.\n"
                      "[test.cpp:2]: (warning) %p in format string (no. 8) requires an address but the argument type is 'signed int'.\n", errout.str());

        check("unsigned int f() { return 0; }\n"
              "void foo() { printf(\"%u %d %ld %I64d %I64u %f %Lf %p\", f(), f(), f(), f(), f(), f(), f(), f()); }");
        ASSERT_EQUALS("[test.cpp:2]: (warning) %d in format string (no. 2) requires 'int' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:2]: (warning) %ld in format string (no. 3) requires 'long' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:2]: (warning) %I64d in format string (no. 4) requires '__int64' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:2]: (warning) %I64u in format string (no. 5) requires 'unsigned __int64' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:2]: (warning) %f in format string (no. 6) requires 'double' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:2]: (warning) %Lf in format string (no. 7) requires 'long double' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:2]: (warning) %p in format string (no. 8) requires an address but the argument type is 'unsigned int'.\n", errout.str());

        check("long f() { return 0; }\n"
              "void foo() { printf(\"%ld %u %lu %I64u %I64d %f %Lf %p\", f(), f(), f(), f(), f(), f(), f(), f()); }");
        ASSERT_EQUALS("[test.cpp:2]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'signed long'.\n"
                      "[test.cpp:2]: (warning) %lu in format string (no. 3) requires 'unsigned long' but the argument type is 'signed long'.\n"
                      "[test.cpp:2]: (warning) %I64u in format string (no. 4) requires 'unsigned __int64' but the argument type is 'signed long'.\n"
                      "[test.cpp:2]: (warning) %I64d in format string (no. 5) requires '__int64' but the argument type is 'signed long'.\n"
                      "[test.cpp:2]: (warning) %f in format string (no. 6) requires 'double' but the argument type is 'signed long'.\n"
                      "[test.cpp:2]: (warning) %Lf in format string (no. 7) requires 'long double' but the argument type is 'signed long'.\n"
                      "[test.cpp:2]: (warning) %p in format string (no. 8) requires an address but the argument type is 'signed long'.\n", errout.str());

        check("unsigned long f() { return 0; }\n"
              "void foo() { printf(\"%lu %d %ld %I64d %I64u %f %Lf %p\", f(), f(), f(), f(), f(), f(), f(), f()); }");
        ASSERT_EQUALS("[test.cpp:2]: (warning) %d in format string (no. 2) requires 'int' but the argument type is 'unsigned long'.\n"
                      "[test.cpp:2]: (warning) %ld in format string (no. 3) requires 'long' but the argument type is 'unsigned long'.\n"
                      "[test.cpp:2]: (warning) %I64d in format string (no. 4) requires '__int64' but the argument type is 'unsigned long'.\n"
                      "[test.cpp:2]: (warning) %I64u in format string (no. 5) requires 'unsigned __int64' but the argument type is 'unsigned long'.\n"
                      "[test.cpp:2]: (warning) %f in format string (no. 6) requires 'double' but the argument type is 'unsigned long'.\n"
                      "[test.cpp:2]: (warning) %Lf in format string (no. 7) requires 'long double' but the argument type is 'unsigned long'.\n"
                      "[test.cpp:2]: (warning) %p in format string (no. 8) requires an address but the argument type is 'unsigned long'.\n", errout.str());

        check("long long f() { return 0; }\n"
              "void foo() { printf(\"%lld %u %lu %I64u %I64d %f %Lf %p\", f(), f(), f(), f(), f(), f(), f(), f()); }");
        ASSERT_EQUALS("[test.cpp:2]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'signed long long'.\n"
                      "[test.cpp:2]: (warning) %lu in format string (no. 3) requires 'unsigned long' but the argument type is 'signed long long'.\n"
                      "[test.cpp:2]: (warning) %I64u in format string (no. 4) requires 'unsigned __int64' but the argument type is 'signed long long'.\n"
                      "[test.cpp:2]: (warning) %f in format string (no. 6) requires 'double' but the argument type is 'signed long long'.\n"
                      "[test.cpp:2]: (warning) %Lf in format string (no. 7) requires 'long double' but the argument type is 'signed long long'.\n"
                      "[test.cpp:2]: (warning) %p in format string (no. 8) requires an address but the argument type is 'signed long long'.\n", errout.str());

        check("unsigned long long f() { return 0; }\n"
              "void foo() { printf(\"%llu %d %ld %I64d %I64u %f %Lf %p\", f(), f(), f(), f(), f(), f(), f(), f()); }");
        ASSERT_EQUALS("[test.cpp:2]: (warning) %d in format string (no. 2) requires 'int' but the argument type is 'unsigned long long'.\n"
                      "[test.cpp:2]: (warning) %ld in format string (no. 3) requires 'long' but the argument type is 'unsigned long long'.\n"
                      "[test.cpp:2]: (warning) %I64d in format string (no. 4) requires '__int64' but the argument type is 'unsigned long long'.\n"
                      "[test.cpp:2]: (warning) %f in format string (no. 6) requires 'double' but the argument type is 'unsigned long long'.\n"
                      "[test.cpp:2]: (warning) %Lf in format string (no. 7) requires 'long double' but the argument type is 'unsigned long long'.\n"
                      "[test.cpp:2]: (warning) %p in format string (no. 8) requires an address but the argument type is 'unsigned long long'.\n", errout.str());

        check("float f() { return 0; }\n"
              "void foo() { printf(\"%f %d %ld %u %lu %I64d %I64u %Lf %p\", f(), f(), f(), f(), f(), f(), f(), f(), f()); }");
        ASSERT_EQUALS("[test.cpp:2]: (warning) %d in format string (no. 2) requires 'int' but the argument type is 'float'.\n"
                      "[test.cpp:2]: (warning) %ld in format string (no. 3) requires 'long' but the argument type is 'float'.\n"
                      "[test.cpp:2]: (warning) %u in format string (no. 4) requires 'unsigned int' but the argument type is 'float'.\n"
                      "[test.cpp:2]: (warning) %lu in format string (no. 5) requires 'unsigned long' but the argument type is 'float'.\n"
                      "[test.cpp:2]: (warning) %I64d in format string (no. 6) requires '__int64' but the argument type is 'float'.\n"
                      "[test.cpp:2]: (warning) %I64u in format string (no. 7) requires 'unsigned __int64' but the argument type is 'float'.\n"
                      "[test.cpp:2]: (warning) %Lf in format string (no. 8) requires 'long double' but the argument type is 'float'.\n"
                      "[test.cpp:2]: (warning) %p in format string (no. 9) requires an address but the argument type is 'float'.\n", errout.str());

        check("double f() { return 0; }\n"
              "void foo() { printf(\"%f %d %ld %u %lu %I64d %I64u %Lf %p\", f(), f(), f(), f(), f(), f(), f(), f(), f()); }");
        ASSERT_EQUALS("[test.cpp:2]: (warning) %d in format string (no. 2) requires 'int' but the argument type is 'double'.\n"
                      "[test.cpp:2]: (warning) %ld in format string (no. 3) requires 'long' but the argument type is 'double'.\n"
                      "[test.cpp:2]: (warning) %u in format string (no. 4) requires 'unsigned int' but the argument type is 'double'.\n"
                      "[test.cpp:2]: (warning) %lu in format string (no. 5) requires 'unsigned long' but the argument type is 'double'.\n"
                      "[test.cpp:2]: (warning) %I64d in format string (no. 6) requires '__int64' but the argument type is 'double'.\n"
                      "[test.cpp:2]: (warning) %I64u in format string (no. 7) requires 'unsigned __int64' but the argument type is 'double'.\n"
                      "[test.cpp:2]: (warning) %Lf in format string (no. 8) requires 'long double' but the argument type is 'double'.\n"
                      "[test.cpp:2]: (warning) %p in format string (no. 9) requires an address but the argument type is 'double'.\n", errout.str());

        check("long double f() { return 0; }\n"
              "void foo() { printf(\"%Lf %d %ld %u %lu %I64d %I64u %f %p\", f(), f(), f(), f(), f(), f(), f(), f(), f()); }");
        ASSERT_EQUALS("[test.cpp:2]: (warning) %d in format string (no. 2) requires 'int' but the argument type is 'long double'.\n"
                      "[test.cpp:2]: (warning) %ld in format string (no. 3) requires 'long' but the argument type is 'long double'.\n"
                      "[test.cpp:2]: (warning) %u in format string (no. 4) requires 'unsigned int' but the argument type is 'long double'.\n"
                      "[test.cpp:2]: (warning) %lu in format string (no. 5) requires 'unsigned long' but the argument type is 'long double'.\n"
                      "[test.cpp:2]: (warning) %I64d in format string (no. 6) requires '__int64' but the argument type is 'long double'.\n"
                      "[test.cpp:2]: (warning) %I64u in format string (no. 7) requires 'unsigned __int64' but the argument type is 'long double'.\n"
                      "[test.cpp:2]: (warning) %f in format string (no. 8) requires 'double' but the argument type is 'long double'.\n"
                      "[test.cpp:2]: (warning) %p in format string (no. 9) requires an address but the argument type is 'long double'.\n", errout.str());

        check("namespace bar { int f() { return 0; } }\n"
              "void foo() { printf(\"%d %u %lu %f %Lf %p\", bar::f(), bar::f(), bar::f(), bar::f(), bar::f(), bar::f()); }");
        ASSERT_EQUALS("[test.cpp:2]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'signed int'.\n"
                      "[test.cpp:2]: (warning) %lu in format string (no. 3) requires 'unsigned long' but the argument type is 'signed int'.\n"
                      "[test.cpp:2]: (warning) %f in format string (no. 4) requires 'double' but the argument type is 'signed int'.\n"
                      "[test.cpp:2]: (warning) %Lf in format string (no. 5) requires 'long double' but the argument type is 'signed int'.\n"
                      "[test.cpp:2]: (warning) %p in format string (no. 6) requires an address but the argument type is 'signed int'.\n", errout.str());

        check("struct Fred { int i; } f;\n"
              "void foo() { printf(\"%d %u %lu %f %Lf %p\", f.i, f.i, f.i, f.i, f.i, f.i); }");
        ASSERT_EQUALS("[test.cpp:2]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'signed int'.\n"
                      "[test.cpp:2]: (warning) %lu in format string (no. 3) requires 'unsigned long' but the argument type is 'signed int'.\n"
                      "[test.cpp:2]: (warning) %f in format string (no. 4) requires 'double' but the argument type is 'signed int'.\n"
                      "[test.cpp:2]: (warning) %Lf in format string (no. 5) requires 'long double' but the argument type is 'signed int'.\n"
                      "[test.cpp:2]: (warning) %p in format string (no. 6) requires an address but the argument type is 'signed int'.\n", errout.str());

        check("struct Fred { unsigned int u; } f;\n"
              "void foo() { printf(\"%u %d %ld %f %Lf %p\", f.u, f.u, f.u, f.u, f.u, f.u); }");
        ASSERT_EQUALS("[test.cpp:2]: (warning) %d in format string (no. 2) requires 'int' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:2]: (warning) %ld in format string (no. 3) requires 'long' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:2]: (warning) %f in format string (no. 4) requires 'double' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:2]: (warning) %Lf in format string (no. 5) requires 'long double' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:2]: (warning) %p in format string (no. 6) requires an address but the argument type is 'unsigned int'.\n", errout.str());

        check("struct Fred { unsigned int ui() { return 0; } } f;\n"
              "void foo() { printf(\"%u %d %ld %f %Lf %p\", f.ui(), f.ui(), f.ui(), f.ui(), f.ui(), f.ui()); }");
        ASSERT_EQUALS("[test.cpp:2]: (warning) %d in format string (no. 2) requires 'int' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:2]: (warning) %ld in format string (no. 3) requires 'long' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:2]: (warning) %f in format string (no. 4) requires 'double' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:2]: (warning) %Lf in format string (no. 5) requires 'long double' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:2]: (warning) %p in format string (no. 6) requires an address but the argument type is 'unsigned int'.\n", errout.str());

        // #4975
        check("void f(int len, int newline) {\n"
              "    printf(\"%s\", newline ? a : str + len);\n"
              "    printf(\"%s\", newline + newline);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (warning) %s in format string (no. 1) requires 'char *' but the argument type is 'signed int'.\n", errout.str());

        check("struct Fred { int i; } f;\n"
              "struct Fred & bar() { };\n"
              "void foo() { printf(\"%d %u %lu %f %Lf %p\", bar().i, bar().i, bar().i, bar().i, bar().i, bar().i); }");
        ASSERT_EQUALS("[test.cpp:3]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'signed int'.\n"
                      "[test.cpp:3]: (warning) %lu in format string (no. 3) requires 'unsigned long' but the argument type is 'signed int'.\n"
                      "[test.cpp:3]: (warning) %f in format string (no. 4) requires 'double' but the argument type is 'signed int'.\n"
                      "[test.cpp:3]: (warning) %Lf in format string (no. 5) requires 'long double' but the argument type is 'signed int'.\n"
                      "[test.cpp:3]: (warning) %p in format string (no. 6) requires an address but the argument type is 'signed int'.\n", errout.str());

        check("struct Fred { int i; } f;\n"
              "const struct Fred & bar() { };\n"
              "void foo() { printf(\"%d %u %lu %f %Lf %p\", bar().i, bar().i, bar().i, bar().i, bar().i, bar().i); }");
        ASSERT_EQUALS("[test.cpp:3]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'signed int'.\n"
                      "[test.cpp:3]: (warning) %lu in format string (no. 3) requires 'unsigned long' but the argument type is 'signed int'.\n"
                      "[test.cpp:3]: (warning) %f in format string (no. 4) requires 'double' but the argument type is 'signed int'.\n"
                      "[test.cpp:3]: (warning) %Lf in format string (no. 5) requires 'long double' but the argument type is 'signed int'.\n"
                      "[test.cpp:3]: (warning) %p in format string (no. 6) requires an address but the argument type is 'signed int'.\n", errout.str());

        check("struct Fred { int i; } f;\n"
              "static const struct Fred & bar() { };\n"
              "void foo() { printf(\"%d %u %lu %f %Lf %p\", bar().i, bar().i, bar().i, bar().i, bar().i, bar().i); }");
        ASSERT_EQUALS("[test.cpp:3]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'signed int'.\n"
                      "[test.cpp:3]: (warning) %lu in format string (no. 3) requires 'unsigned long' but the argument type is 'signed int'.\n"
                      "[test.cpp:3]: (warning) %f in format string (no. 4) requires 'double' but the argument type is 'signed int'.\n"
                      "[test.cpp:3]: (warning) %Lf in format string (no. 5) requires 'long double' but the argument type is 'signed int'.\n"
                      "[test.cpp:3]: (warning) %p in format string (no. 6) requires an address but the argument type is 'signed int'.\n", errout.str());

        check("struct Fred { int i; } f[2];\n"
              "struct Fred * bar() { return f; };\n"
              "void foo() { printf(\"%d %u %lu %f %Lf %p\", bar()[0].i, bar()[0].i, bar()[0].i, bar()[0].i, bar()[0].i, bar()[0].i); }");
        ASSERT_EQUALS("[test.cpp:3]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'signed int'.\n"
                      "[test.cpp:3]: (warning) %lu in format string (no. 3) requires 'unsigned long' but the argument type is 'signed int'.\n"
                      "[test.cpp:3]: (warning) %f in format string (no. 4) requires 'double' but the argument type is 'signed int'.\n"
                      "[test.cpp:3]: (warning) %Lf in format string (no. 5) requires 'long double' but the argument type is 'signed int'.\n"
                      "[test.cpp:3]: (warning) %p in format string (no. 6) requires an address but the argument type is 'signed int'.\n", errout.str());

        check("struct Fred { int i; } f[2];\n"
              "const struct Fred * bar() { return f; };\n"
              "void foo() { printf(\"%d %u %lu %f %Lf %p\", bar()[0].i, bar()[0].i, bar()[0].i, bar()[0].i, bar()[0].i, bar()[0].i); }");
        ASSERT_EQUALS("[test.cpp:3]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'signed int'.\n"
                      "[test.cpp:3]: (warning) %lu in format string (no. 3) requires 'unsigned long' but the argument type is 'signed int'.\n"
                      "[test.cpp:3]: (warning) %f in format string (no. 4) requires 'double' but the argument type is 'signed int'.\n"
                      "[test.cpp:3]: (warning) %Lf in format string (no. 5) requires 'long double' but the argument type is 'signed int'.\n"
                      "[test.cpp:3]: (warning) %p in format string (no. 6) requires an address but the argument type is 'signed int'.\n", errout.str());

        check("struct Fred { int i; } f[2];\n"
              "static const struct Fred * bar() { return f; };\n"
              "void foo() { printf(\"%d %u %lu %f %Lf %p\", bar()[0].i, bar()[0].i, bar()[0].i, bar()[0].i, bar()[0].i, bar()[0].i); }");
        ASSERT_EQUALS("[test.cpp:3]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'signed int'.\n"
                      "[test.cpp:3]: (warning) %lu in format string (no. 3) requires 'unsigned long' but the argument type is 'signed int'.\n"
                      "[test.cpp:3]: (warning) %f in format string (no. 4) requires 'double' but the argument type is 'signed int'.\n"
                      "[test.cpp:3]: (warning) %Lf in format string (no. 5) requires 'long double' but the argument type is 'signed int'.\n"
                      "[test.cpp:3]: (warning) %p in format string (no. 6) requires an address but the argument type is 'signed int'.\n", errout.str());

        check("struct Fred { int32_t i; } f;\n"
              "struct Fred & bar() { };\n"
              "void foo() { printf(\"%d %ld %u %lu %f %Lf\", bar().i, bar().i, bar().i, bar().i, bar().i, bar().i); }");
        ASSERT_EQUALS("[test.cpp:3]: (warning) %ld in format string (no. 2) requires 'long' but the argument type is 'signed int'.\n"
                      "[test.cpp:3]: (warning) %u in format string (no. 3) requires 'unsigned int' but the argument type is 'signed int'.\n"
                      "[test.cpp:3]: (warning) %lu in format string (no. 4) requires 'unsigned long' but the argument type is 'signed int'.\n"
                      "[test.cpp:3]: (warning) %f in format string (no. 5) requires 'double' but the argument type is 'signed int'.\n"
                      "[test.cpp:3]: (warning) %Lf in format string (no. 6) requires 'long double' but the argument type is 'signed int'.\n",
                      errout.str());

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
        ASSERT_EQUALS("[test.cpp:4]: (warning) %f in format string (no. 1) requires 'double' but the argument type is 'signed int'.\n", errout.str());

        check("struct Base { int length() { } };\n"
              "struct Derived : public Base { };\n"
              "void foo(Derived * d) {\n"
              "    printf(\"%f\", d.length());\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (warning) %f in format string (no. 1) requires 'double' but the argument type is 'signed int'.\n", errout.str());

        check("std::vector<int> v;\n"
              "void foo() {\n"
              "    printf(\"%d %u %f\", v[0], v[0], v[0]);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'int'.\n"
                      "[test.cpp:3]: (warning) %f in format string (no. 3) requires 'double' but the argument type is 'int'.\n", errout.str());

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
              "}\n", false, true, Settings::Win32A);
        ASSERT_EQUALS("[test.cpp:4]: (portability) %d in format string (no. 3) requires 'int' but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:4]: (portability) %f in format string (no. 4) requires 'double' but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:5]: (portability) %d in format string (no. 3) requires 'int' but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:5]: (portability) %f in format string (no. 4) requires 'double' but the argument type is 'size_t {aka unsigned long}'.\n", errout.str());

        check("std::vector<int> v;\n"
              "std::string s;\n"
              "void foo() {\n"
              "    printf(\"%zu %Iu %d %f\", v.size(), v.size(), v.size(), v.size());\n"
              "    printf(\"%zu %Iu %d %f\", s.size(), s.size(), s.size(), s.size());\n"
              "}\n", false, true, Settings::Win64);
        ASSERT_EQUALS("[test.cpp:4]: (portability) %d in format string (no. 3) requires 'int' but the argument type is 'size_t {aka unsigned long long}'.\n"
                      "[test.cpp:4]: (portability) %f in format string (no. 4) requires 'double' but the argument type is 'size_t {aka unsigned long long}'.\n"
                      "[test.cpp:5]: (portability) %d in format string (no. 3) requires 'int' but the argument type is 'size_t {aka unsigned long long}'.\n"
                      "[test.cpp:5]: (portability) %f in format string (no. 4) requires 'double' but the argument type is 'size_t {aka unsigned long long}'.\n", errout.str());

        check("std::vector<int> v;\n"
              "std::string s;\n"
              "void foo() {\n"
              "    printf(\"%zu %Iu %d %f\", v.size(), v.size(), v.size(), v.size());\n"
              "    printf(\"%zu %Iu %d %f\", s.size(), s.size(), s.size(), s.size());\n"
              "}\n", false, true, Settings::Unix32);
        ASSERT_EQUALS("[test.cpp:4]: (portability) %d in format string (no. 3) requires 'int' but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:4]: (portability) %f in format string (no. 4) requires 'double' but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:5]: (portability) %d in format string (no. 3) requires 'int' but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:5]: (portability) %f in format string (no. 4) requires 'double' but the argument type is 'size_t {aka unsigned long}'.\n", errout.str());

        check("std::vector<int> v;\n"
              "std::string s;\n"
              "void foo() {\n"
              "    printf(\"%zu %Iu %d %f\", v.size(), v.size(), v.size(), v.size());\n"
              "    printf(\"%zu %Iu %d %f\", s.size(), s.size(), s.size(), s.size());\n"
              "}\n", false, true, Settings::Unix64);
        ASSERT_EQUALS("[test.cpp:4]: (portability) %d in format string (no. 3) requires 'int' but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:4]: (portability) %f in format string (no. 4) requires 'double' but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:5]: (portability) %d in format string (no. 3) requires 'int' but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:5]: (portability) %f in format string (no. 4) requires 'double' but the argument type is 'size_t {aka unsigned long}'.\n", errout.str());

        check("class Fred : public std::vector<int> {} v;\n"
              "std::string s;\n"
              "void foo() {\n"
              "    printf(\"%zu %Iu %d %f\", v.size(), v.size(), v.size(), v.size());\n"
              "    printf(\"%zu %Iu %d %f\", s.size(), s.size(), s.size(), s.size());\n"
              "}\n", false, true, Settings::Unix64);
        ASSERT_EQUALS("[test.cpp:4]: (portability) %d in format string (no. 3) requires 'int' but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:4]: (portability) %f in format string (no. 4) requires 'double' but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:5]: (portability) %d in format string (no. 3) requires 'int' but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:5]: (portability) %f in format string (no. 4) requires 'double' but the argument type is 'size_t {aka unsigned long}'.\n", errout.str());

        check("class Fred : public std::vector<int> {} v;\n"
              "void foo() {\n"
              "    printf(\"%d %u %f\", v[0], v[0], v[0]);\n"
              "}\n", false, true, Settings::Unix64);
        ASSERT_EQUALS("[test.cpp:3]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'int'.\n"
                      "[test.cpp:3]: (warning) %f in format string (no. 3) requires 'double' but the argument type is 'int'.\n", errout.str());

        check("std::string s;\n"
              "void foo() {\n"
              "    printf(\"%s %p %u %d %f\", s.c_str(), s.c_str(), s.c_str(), s.c_str(), s.c_str());\n"
              "}\n", false, true, Settings::Unix64);
        ASSERT_EQUALS("[test.cpp:3]: (warning) %u in format string (no. 3) requires 'unsigned int' but the argument type is 'const char *'.\n"
                      "[test.cpp:3]: (warning) %d in format string (no. 4) requires 'int' but the argument type is 'const char *'.\n"
                      "[test.cpp:3]: (warning) %f in format string (no. 5) requires 'double' but the argument type is 'const char *'.\n", errout.str());

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
              "}\n", false, true, Settings::Unix64);
        ASSERT_EQUALS("[test.cpp:8]: (warning) %u in format string (no. 1) requires 'unsigned int' but the argument type is 'char *'.\n"
                      "[test.cpp:8]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'char *'.\n"
                      "[test.cpp:8]: (warning) %u in format string (no. 3) requires 'unsigned int' but the argument type is 'char *'.\n"
                      "[test.cpp:9]: (portability) %u in format string (no. 1) requires 'unsigned int' but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:9]: (portability) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:10]: (portability) %lu in format string (no. 1) requires 'unsigned long' but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:10]: (portability) %lu in format string (no. 2) requires 'unsigned long' but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:11]: (portability) %llu in format string (no. 1) requires 'unsigned long long' but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:11]: (portability) %llu in format string (no. 2) requires 'unsigned long long' but the argument type is 'size_t {aka unsigned long}'.\n", errout.str());

        check("bool b; bool bf();\n"
              "char c; char cf();\n"
              "signed char sc; signed char scf();\n"
              "unsigned char uc; unsigned char ucf();\n"
              "short s; short sf();\n"
              "unsigned short us; unsigned short usf();\n"
              "size_t st; size_t stf();\n"
              "ptrdiff_t pt; ptrdiff_t ptf();\n"
              "char * pc; char * pcf();\n"
              "char cl[] = \"123\";\n"
              "char ca[3];\n"
              "void foo() {\n"
              "    printf(\"%td %zd %d %d %d %d %d %d %d %d %d %d %d\", pt, pt, b, c, sc, uc, s, us, st, pt, pc, cl, ca);\n"
              "}\n", false, true, Settings::Unix64);
        ASSERT_EQUALS("[test.cpp:13]: (portability) %zd in format string (no. 2) requires 'ssize_t' but the argument type is 'ptrdiff_t {aka signed long}'.\n"
                      "[test.cpp:13]: (portability) %d in format string (no. 9) requires 'int' but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:13]: (portability) %d in format string (no. 10) requires 'int' but the argument type is 'ptrdiff_t {aka signed long}'.\n"
                      "[test.cpp:13]: (warning) %d in format string (no. 11) requires 'int' but the argument type is 'char *'.\n"
                      "[test.cpp:13]: (warning) %d in format string (no. 12) requires 'int' but the argument type is 'char *'.\n"
                      "[test.cpp:13]: (warning) %d in format string (no. 13) requires 'int' but the argument type is 'char *'.\n", errout.str());

        check("bool b; bool bf();\n"
              "char c; char cf();\n"
              "signed char sc; signed char scf();\n"
              "unsigned char uc; unsigned char ucf();\n"
              "short s; short sf();\n"
              "unsigned short us; unsigned short usf();\n"
              "size_t st; size_t stf();\n"
              "ptrdiff_t pt; ptrdiff_t ptf();\n"
              "char * pc; char * pcf();\n"
              "char cl[] = \"123\";\n"
              "char ca[3];\n"
              "void foo() {\n"
              "    printf(\"%ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld\", b, c, sc, uc, s, us, st, pt, pc, cl, ca);\n"
              "}\n", false, true, Settings::Unix64);
        ASSERT_EQUALS("[test.cpp:13]: (warning) %ld in format string (no. 1) requires 'long' but the argument type is 'bool'.\n"
                      "[test.cpp:13]: (warning) %ld in format string (no. 2) requires 'long' but the argument type is 'char'.\n"
                      "[test.cpp:13]: (warning) %ld in format string (no. 3) requires 'long' but the argument type is 'signed char'.\n"
                      "[test.cpp:13]: (warning) %ld in format string (no. 4) requires 'long' but the argument type is 'unsigned char'.\n"
                      "[test.cpp:13]: (warning) %ld in format string (no. 5) requires 'long' but the argument type is 'signed short'.\n"
                      "[test.cpp:13]: (warning) %ld in format string (no. 6) requires 'long' but the argument type is 'unsigned short'.\n"
                      "[test.cpp:13]: (portability) %ld in format string (no. 7) requires 'long' but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:13]: (portability) %ld in format string (no. 8) requires 'long' but the argument type is 'ptrdiff_t {aka signed long}'.\n"
                      "[test.cpp:13]: (warning) %ld in format string (no. 9) requires 'long' but the argument type is 'char *'.\n"
                      "[test.cpp:13]: (warning) %ld in format string (no. 10) requires 'long' but the argument type is 'char *'.\n"
                      "[test.cpp:13]: (warning) %ld in format string (no. 11) requires 'long' but the argument type is 'char *'.\n", errout.str());


        check("bool b; bool bf();\n"
              "char c; char cf();\n"
              "signed char sc; signed char scf();\n"
              "unsigned char uc; unsigned char ucf();\n"
              "short s; short sf();\n"
              "unsigned short us; unsigned short usf();\n"
              "size_t st; size_t stf();\n"
              "ptrdiff_t pt; ptrdiff_t ptf();\n"
              "char * pc; char * pcf();\n"
              "char cl[] = \"123\";\n"
              "char ca[3];\n"
              "void foo() {\n"
              "    printf(\"%td %zd %d %d %d %d %d %d %d %d %d\", ptf(), ptf(), bf(), cf(), scf(), ucf(), sf(), usf(), stf(), ptf(), pcf());\n"
              "}\n", false, true, Settings::Unix64);
        ASSERT_EQUALS("[test.cpp:13]: (portability) %zd in format string (no. 2) requires 'ssize_t' but the argument type is 'ptrdiff_t {aka signed long}'.\n"
                      "[test.cpp:13]: (portability) %d in format string (no. 9) requires 'int' but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:13]: (portability) %d in format string (no. 10) requires 'int' but the argument type is 'ptrdiff_t {aka signed long}'.\n"
                      "[test.cpp:13]: (warning) %d in format string (no. 11) requires 'int' but the argument type is 'char *'.\n", errout.str());

        check("bool b; bool bf();\n"
              "char c; char cf();\n"
              "signed char sc; signed char scf();\n"
              "unsigned char uc; unsigned char ucf();\n"
              "short s; short sf();\n"
              "unsigned short us; unsigned short usf();\n"
              "size_t st; size_t stf();\n"
              "ptrdiff_t pt; ptrdiff_t ptf();\n"
              "char * pc; char * pcf();\n"
              "char cl[] = \"123\";\n"
              "char ca[3];\n"
              "void foo() {\n"
              "    printf(\"%ld %ld %ld %ld %ld %ld %ld %ld %ld\", bf(), cf(), scf(), ucf(), sf(), usf(), stf(), ptf(), pcf());\n"
              "}\n", false, true, Settings::Unix64);
        ASSERT_EQUALS("[test.cpp:13]: (warning) %ld in format string (no. 1) requires 'long' but the argument type is 'bool'.\n"
                      "[test.cpp:13]: (warning) %ld in format string (no. 2) requires 'long' but the argument type is 'char'.\n"
                      "[test.cpp:13]: (warning) %ld in format string (no. 3) requires 'long' but the argument type is 'signed char'.\n"
                      "[test.cpp:13]: (warning) %ld in format string (no. 4) requires 'long' but the argument type is 'unsigned char'.\n"
                      "[test.cpp:13]: (warning) %ld in format string (no. 5) requires 'long' but the argument type is 'signed short'.\n"
                      "[test.cpp:13]: (warning) %ld in format string (no. 6) requires 'long' but the argument type is 'unsigned short'.\n"
                      "[test.cpp:13]: (portability) %ld in format string (no. 7) requires 'long' but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:13]: (portability) %ld in format string (no. 8) requires 'long' but the argument type is 'ptrdiff_t {aka signed long}'.\n"
                      "[test.cpp:13]: (warning) %ld in format string (no. 9) requires 'long' but the argument type is 'char *'.\n", errout.str());

        check("struct A {};\n"
              "class B : public std::vector<const int *> {} b;\n"
              "class C : public std::vector<const struct A *> {} c;\n"
              "std::string s;\n"
              "void foo() {\n"
              "    printf(\"%zu %u\", b.size(), b.size());\n"
              "    printf(\"%p %d\", b[0], b[0]);\n"
              "    printf(\"%p %d\", c[0], c[0]);\n"
              "    printf(\"%p %d\", s.c_str(), s.c_str());\n"
              "}\n", false, true, Settings::Unix64);
        ASSERT_EQUALS("[test.cpp:6]: (portability) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:7]: (warning) %d in format string (no. 2) requires 'int' but the argument type is 'const int *'.\n"
                      "[test.cpp:8]: (warning) %d in format string (no. 2) requires 'int' but the argument type is 'const struct A *'.\n"
                      "[test.cpp:9]: (warning) %d in format string (no. 2) requires 'int' but the argument type is 'const char *'.\n", errout.str());

        check("class A : public std::vector<std::string> {} a;\n"
              "class B : public std::string {} b;\n"
              "std::string s;\n"
              "void foo() {\n"
              "    printf(\"%p %d\", a[0].c_str(), a[0].c_str());\n"
              "    printf(\"%c %p\", b[0], b[0]);\n"
              "    printf(\"%c %p\", s[0], s[0]);\n"
              "}\n", false, false, Settings::Unix64);
        ASSERT_EQUALS("[test.cpp:5]: (warning) %d in format string (no. 2) requires 'int' but the argument type is 'const char *'.\n"
                      "[test.cpp:6]: (warning) %p in format string (no. 2) requires an address but the argument type is 'char'.\n"
                      "[test.cpp:7]: (warning) %p in format string (no. 2) requires an address but the argument type is 'char'.\n", errout.str());

        check("template <class T>\n"
              "struct buffer {\n"
              "    size_t size();\n"
              "};\n"
              "buffer<int> b;\n"
              "void foo() {\n"
              "    printf(\"%u\", b.size());\n"
              "}\n", false, true, Settings::Unix64);
        ASSERT_EQUALS("[test.cpp:7]: (portability) %u in format string (no. 1) requires 'unsigned int' but the argument type is 'size_t {aka unsigned long}'.\n", errout.str());

        check("DWORD a;\n"
              "DWORD_PTR b;\n"
              "void foo() {\n"
              "    printf(\"%u %u\", a, b);\n"
              "}\n", false, true, Settings::Win32A);
        ASSERT_EQUALS("[test.cpp:4]: (portability) %u in format string (no. 1) requires 'unsigned int' but the argument type is 'DWORD {aka unsigned long}'.\n"
                      "[test.cpp:4]: (portability) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'DWORD_PTR {aka unsigned long}'.\n", errout.str());

        check("unsigned long a[] = { 1, 2 };\n"
              "void foo() {\n"
              "    printf(\"%d %d %x \", a[0], a[0], a[0]);\n"
              "}\n", false, false, Settings::Win32A);
        ASSERT_EQUALS("[test.cpp:3]: (warning) %d in format string (no. 1) requires 'int' but the argument type is 'unsigned long'.\n"
                      "[test.cpp:3]: (warning) %d in format string (no. 2) requires 'int' but the argument type is 'unsigned long'.\n"
                      "[test.cpp:3]: (warning) %x in format string (no. 3) requires 'unsigned int' but the argument type is 'unsigned long'.\n", errout.str());

        check("void foo (wchar_t c) {\n" // ticket #5051 false positive
              "    printf(\"%c\", c);\n"
              "}\n", false, false, Settings::Win64);
        ASSERT_EQUALS("", errout.str());

        check("void foo() {\n"
              "    printf(\"%f %d\", static_cast<int>(1.0f), reinterpret_cast<const void *>(0));\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2]: (warning) %f in format string (no. 1) requires 'double' but the argument type is 'signed int'.\n"
                      "[test.cpp:2]: (warning) %d in format string (no. 2) requires 'int' but the argument type is 'const void *'.\n", errout.str());

        check("void foo() {\n"
              "    UNKNOWN * u;\n"
              "    printf(\"%d %x %u %f\", u[i], u[i], u[i], u[i]);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void foo() {\n"
              "    long * l;\n"
              "    printf(\"%d %x %u %f\", l[i], l[i], l[i], l[i]);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (warning) %d in format string (no. 1) requires 'int' but the argument type is 'signed long'.\n"
                      "[test.cpp:3]: (warning) %x in format string (no. 2) requires 'unsigned int' but the argument type is 'signed long'.\n"
                      "[test.cpp:3]: (warning) %u in format string (no. 3) requires 'unsigned int' but the argument type is 'signed long'.\n"
                      "[test.cpp:3]: (warning) %f in format string (no. 4) requires 'double' but the argument type is 'signed long'.\n", errout.str());

        check("void f() {\n" // #5104
              "    myvector<unsigned short> v1(1,0);\n"
              "    printf(\"%d\n\",v1[0]);\n"
              "    myvector<int> v2(1,0);\n"
              "    printf(\"%d\n\",v2[0]);\n"
              "    myvector<unsigned int> v3(1,0);\n"
              "    printf(\"%u\n\",v3[0]);\n"
              "    myvector<unsigned int> v4(1,0);\n"
              "    printf(\"%x\n\",v4[0]);\n"
              "    myvector<double> v5(1,0);\n"
              "    printf(\"%f\n\",v5[0]);\n"
              "    myvector<bool> v6(1,0);\n"
              "    printf(\"%u\n\",v6[0]);\n"
              "    myvector<char *> v7(1,0);\n"
              "    printf(\"%s\n\",v7[0]);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("std::vector<char> v;\n" // #5151
              "void foo() {\n"
              "   printf(\"%c %u %f\", v.at(32), v.at(32), v.at(32));\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:3]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'char'.\n"
                      "[test.cpp:3]: (warning) %f in format string (no. 3) requires 'double' but the argument type is 'char'.\n", errout.str());

        // #5195 (segmentation fault)
        check("void T::a(const std::vector<double>& vx) {\n"
              "    printf(\"%f\", vx.at(0));\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        // #5486
        check("void foo() {\n"
              "    ssize_t test = 0;\n"
              "    printf(\"%zd\", test);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        // #6009
        check("extern std::string StringByReturnValue();\n"
              "extern int         IntByReturnValue();\n"
              "void MyFunction() {\n"
              "    printf( \"%s - %s\", StringByReturnValue(), IntByReturnValue() );\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (warning) %s in format string (no. 1) requires 'char *' but the argument type is 'std::string'.\n"
                      "[test.cpp:4]: (warning) %s in format string (no. 2) requires 'char *' but the argument type is 'signed int'.\n", errout.str());

        check("template <class T, size_t S>\n"
              "struct Array {\n"
              "    T data[S];\n"
              "    T & operator [] (size_t i) { return data[i]; }\n"
              "};\n"
              "void foo() {\n"
              "    Array<int, 10> array1;\n"
              "    Array<float, 10> array2;\n"
              "    printf(\"%u %u\", array1[0], array2[0]);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:9]: (warning) %u in format string (no. 1) requires 'unsigned int' but the argument type is 'int'.\n"
                      "[test.cpp:9]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'float'.\n", errout.str());

        // Ticket #7445
        check("struct S { unsigned short x; } s = {0};\n"
              "void foo() {\n"
              "    printf(\"%d\", s.x);\n"
              "}");
        ASSERT_EQUALS("", errout.str());

        // Ticket #7601
        check("void foo(int i, unsigned int ui, long long ll, unsigned long long ull) {\n"
              "    printf(\"%Ld %Lu %Ld %Lu\", i, ui, ll, ull);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) %Ld in format string (no. 1) requires 'long long' but the argument type is 'signed int'.\n"
                      "[test.cpp:2]: (warning) %Lu in format string (no. 2) requires 'unsigned long long' but the argument type is 'unsigned int'.\n", errout.str());

        check("void foo(char c, unsigned char uc, short s, unsigned short us, int i, unsigned int ui, long l, unsigned long ul) {\n"
              "    printf(\"%hhd %hhd %hhd %hhd %hhd %hhd %hhd %hhd\", c, uc, s, us, i, ui, l, ul);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) %hhd in format string (no. 2) requires 'char' but the argument type is 'unsigned char'.\n"
                      "[test.cpp:2]: (warning) %hhd in format string (no. 3) requires 'char' but the argument type is 'signed short'.\n"
                      "[test.cpp:2]: (warning) %hhd in format string (no. 4) requires 'char' but the argument type is 'unsigned short'.\n"
                      "[test.cpp:2]: (warning) %hhd in format string (no. 5) requires 'char' but the argument type is 'signed int'.\n"
                      "[test.cpp:2]: (warning) %hhd in format string (no. 6) requires 'char' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:2]: (warning) %hhd in format string (no. 7) requires 'char' but the argument type is 'signed long'.\n"
                      "[test.cpp:2]: (warning) %hhd in format string (no. 8) requires 'char' but the argument type is 'unsigned long'.\n", errout.str());

        check("void foo(char c, unsigned char uc, short s, unsigned short us, int i, unsigned int ui, long l, unsigned long ul) {\n"
              "    printf(\"%hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu\", c, uc, s, us, i, ui, l, ul);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) %hhu in format string (no. 1) requires 'unsigned char' but the argument type is 'char'.\n"
                      "[test.cpp:2]: (warning) %hhu in format string (no. 3) requires 'unsigned char' but the argument type is 'signed short'.\n"
                      "[test.cpp:2]: (warning) %hhu in format string (no. 4) requires 'unsigned char' but the argument type is 'unsigned short'.\n"
                      "[test.cpp:2]: (warning) %hhu in format string (no. 5) requires 'unsigned char' but the argument type is 'signed int'.\n"
                      "[test.cpp:2]: (warning) %hhu in format string (no. 6) requires 'unsigned char' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:2]: (warning) %hhu in format string (no. 7) requires 'unsigned char' but the argument type is 'signed long'.\n"
                      "[test.cpp:2]: (warning) %hhu in format string (no. 8) requires 'unsigned char' but the argument type is 'unsigned long'.\n", errout.str());

        check("void foo(char c, unsigned char uc, short s, unsigned short us, int i, unsigned int ui, long l, unsigned long ul) {\n"
              "    printf(\"%hhx %hhx %hhx %hhx %hhx %hhx %hhx %hhx\", c, uc, s, us, i, ui, l, ul);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) %hhx in format string (no. 3) requires 'unsigned char' but the argument type is 'signed short'.\n"
                      "[test.cpp:2]: (warning) %hhx in format string (no. 4) requires 'unsigned char' but the argument type is 'unsigned short'.\n"
                      "[test.cpp:2]: (warning) %hhx in format string (no. 5) requires 'unsigned char' but the argument type is 'signed int'.\n"
                      "[test.cpp:2]: (warning) %hhx in format string (no. 6) requires 'unsigned char' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:2]: (warning) %hhx in format string (no. 7) requires 'unsigned char' but the argument type is 'signed long'.\n"
                      "[test.cpp:2]: (warning) %hhx in format string (no. 8) requires 'unsigned char' but the argument type is 'unsigned long'.\n", errout.str());

        check("void foo(char c, unsigned char uc, short s, unsigned short us, int i, unsigned int ui, long l, unsigned long ul) {\n"
              "    printf(\"%hd %hd %hd %hd %hd %hd %hd %hd\", c, uc, s, us, i, ui, l, ul);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) %hd in format string (no. 1) requires 'short' but the argument type is 'char'.\n"
                      "[test.cpp:2]: (warning) %hd in format string (no. 2) requires 'short' but the argument type is 'unsigned char'.\n"
                      "[test.cpp:2]: (warning) %hd in format string (no. 4) requires 'short' but the argument type is 'unsigned short'.\n"
                      "[test.cpp:2]: (warning) %hd in format string (no. 5) requires 'short' but the argument type is 'signed int'.\n"
                      "[test.cpp:2]: (warning) %hd in format string (no. 6) requires 'short' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:2]: (warning) %hd in format string (no. 7) requires 'short' but the argument type is 'signed long'.\n"
                      "[test.cpp:2]: (warning) %hd in format string (no. 8) requires 'short' but the argument type is 'unsigned long'.\n", errout.str());

        check("void foo(char c, unsigned char uc, short s, unsigned short us, int i, unsigned int ui, long l, unsigned long ul) {\n"
              "    printf(\"%hu %hu %hu %hu %hu %hu %hu %hu\", c, uc, s, us, i, ui, l, ul);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) %hu in format string (no. 1) requires 'unsigned short' but the argument type is 'char'.\n"
                      "[test.cpp:2]: (warning) %hu in format string (no. 2) requires 'unsigned short' but the argument type is 'unsigned char'.\n"
                      "[test.cpp:2]: (warning) %hu in format string (no. 3) requires 'unsigned short' but the argument type is 'signed short'.\n"
                      "[test.cpp:2]: (warning) %hu in format string (no. 5) requires 'unsigned short' but the argument type is 'signed int'.\n"
                      "[test.cpp:2]: (warning) %hu in format string (no. 6) requires 'unsigned short' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:2]: (warning) %hu in format string (no. 7) requires 'unsigned short' but the argument type is 'signed long'.\n"
                      "[test.cpp:2]: (warning) %hu in format string (no. 8) requires 'unsigned short' but the argument type is 'unsigned long'.\n", errout.str());

        check("void foo(char c, unsigned char uc, short s, unsigned short us, int i, unsigned int ui, long l, unsigned long ul) {\n"
              "    printf(\"%hx %hx %hx %hx %hx %hx %hx %hx\", c, uc, s, us, i, ui, l, ul);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) %hx in format string (no. 1) requires 'unsigned short' but the argument type is 'char'.\n"
                      "[test.cpp:2]: (warning) %hx in format string (no. 2) requires 'unsigned short' but the argument type is 'unsigned char'.\n"
                      "[test.cpp:2]: (warning) %hx in format string (no. 5) requires 'unsigned short' but the argument type is 'signed int'.\n"
                      "[test.cpp:2]: (warning) %hx in format string (no. 6) requires 'unsigned short' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:2]: (warning) %hx in format string (no. 7) requires 'unsigned short' but the argument type is 'signed long'.\n"
                      "[test.cpp:2]: (warning) %hx in format string (no. 8) requires 'unsigned short' but the argument type is 'unsigned long'.\n", errout.str());

        // #7837 - Use ValueType for function call
        check("struct S {\n"
              "  double (* f)(double);\n"
              "};\n"
              "\n"
              "void foo(struct S x) {\n"
              "  printf(\"%f\", x.f(4.0));\n"
              "}");
        ASSERT_EQUALS("", errout.str());
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
        ASSERT_EQUALS("[test.cpp:3]: (error) printf format string requires 1 parameter but only 0 are given.\n"
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
              "}", false, true, Settings::Win32A);
        ASSERT_EQUALS("[test.cpp:8]: (portability) %Id in format string (no. 1) requires 'ptrdiff_t' but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:9]: (portability) %Iu in format string (no. 2) requires 'size_t' but the argument type is 'ptrdiff_t {aka signed long}'.\n"
                      "[test.cpp:10]: (portability) %I32u in format string (no. 2) requires 'unsigned __int32' but the argument type is '__int32 {aka signed int}'.\n"
                      "[test.cpp:11]: (portability) %I32d in format string (no. 1) requires '__int32' but the argument type is 'unsigned __int32 {aka unsigned int}'.\n"
                      "[test.cpp:12]: (portability) %I64u in format string (no. 2) requires 'unsigned __int64' but the argument type is '__int64 {aka signed long long}'.\n"
                      "[test.cpp:13]: (portability) %I64d in format string (no. 1) requires '__int64' but the argument type is 'unsigned __int64 {aka unsigned long long}'.\n", errout.str());

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
              "}", false, true, Settings::Win64);
        ASSERT_EQUALS("[test.cpp:8]: (portability) %Id in format string (no. 1) requires 'ptrdiff_t' but the argument type is 'size_t {aka unsigned long long}'.\n"
                      "[test.cpp:9]: (portability) %Iu in format string (no. 2) requires 'size_t' but the argument type is 'ptrdiff_t {aka signed long long}'.\n"
                      "[test.cpp:10]: (portability) %I32u in format string (no. 2) requires 'unsigned __int32' but the argument type is '__int32 {aka signed int}'.\n"
                      "[test.cpp:11]: (portability) %I32d in format string (no. 1) requires '__int32' but the argument type is 'unsigned __int32 {aka unsigned int}'.\n"
                      "[test.cpp:12]: (portability) %I64u in format string (no. 2) requires 'unsigned __int64' but the argument type is '__int64 {aka signed long long}'.\n"
                      "[test.cpp:13]: (portability) %I64d in format string (no. 1) requires '__int64' but the argument type is 'unsigned __int64 {aka unsigned long long}'.\n", errout.str());

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

        // ticket #5264
        check("void foo(LPARAM lp, WPARAM wp, LRESULT lr) {\n"
              "    printf(\"%Ix %Ix %Ix\", lp, wp, lr);\n"
              "}\n", false, true, Settings::Win64);
        ASSERT_EQUALS("", errout.str());

        check("void foo(LPARAM lp, WPARAM wp, LRESULT lr) {\n"
              "    printf(\"%Ix %Ix %Ix\", lp, wp, lr);\n"
              "}\n", false, true, Settings::Win32A);
        ASSERT_EQUALS("", errout.str());

        check("void foo(UINT32 a, ::UINT32 b, Fred::UINT32 c) {\n"
              "    printf(\"%d %d %d\n\", a, b, c);\n"
              "};\n", false, true, Settings::Win32A);
        ASSERT_EQUALS("[test.cpp:2]: (portability) %d in format string (no. 1) requires 'int' but the argument type is 'UINT32 {aka unsigned int}'.\n"
                      "[test.cpp:2]: (portability) %d in format string (no. 2) requires 'int' but the argument type is 'UINT32 {aka unsigned int}'.\n", errout.str());

        check("void foo(LPCVOID a, ::LPCVOID b, Fred::LPCVOID c) {\n"
              "    printf(\"%d %d %d\n\", a, b, c);\n"
              "};\n", false, true, Settings::Win32A);
        ASSERT_EQUALS("[test.cpp:2]: (warning) %d in format string (no. 1) requires 'int' but the argument type is 'const void *'.\n"
                      "[test.cpp:2]: (warning) %d in format string (no. 2) requires 'int' but the argument type is 'const void *'.\n", errout.str());

    }

    void testMicrosoftScanfArgument() {
        check("void foo() {\n"
              "    size_t s;\n"
              "    ptrdiff_t p;\n"
              "    __int32 i32;\n"
              "    unsigned __int32 u32;\n"
              "    __int64 i64;\n"
              "    unsigned __int64 u64;\n"
              "    scanf(\"%Id %Iu %Ix\", &s, &s, &s);\n"
              "    scanf(\"%Id %Iu %Ix\", &p, &p, &p);\n"
              "    scanf(\"%I32d %I32u %I32x\", &i32, &i32, &i32);\n"
              "    scanf(\"%I32d %I32u %I32x\", &u32, &u32, &u32);\n"
              "    scanf(\"%I64d %I64u %I64x\", &i64, &i64, &i64);\n"
              "    scanf(\"%I64d %I64u %I64x\", &u64, &u64, &u64);\n"
              "}", false, true, Settings::Win32A);
        ASSERT_EQUALS("[test.cpp:8]: (portability) %Id in format string (no. 1) requires 'ptrdiff_t *' but the argument type is 'size_t * {aka unsigned long *}'.\n"
                      "[test.cpp:9]: (portability) %Iu in format string (no. 2) requires 'size_t *' but the argument type is 'ptrdiff_t * {aka signed long *}'.\n"
                      "[test.cpp:10]: (portability) %I32u in format string (no. 2) requires 'unsigned __int32 *' but the argument type is '__int32 * {aka signed int *}'.\n"
                      "[test.cpp:11]: (portability) %I32d in format string (no. 1) requires '__int32 *' but the argument type is 'unsigned __int32 * {aka unsigned int *}'.\n"
                      "[test.cpp:12]: (portability) %I64u in format string (no. 2) requires 'unsigned __int64 *' but the argument type is '__int64 * {aka signed long long *}'.\n"
                      "[test.cpp:13]: (portability) %I64d in format string (no. 1) requires '__int64 *' but the argument type is 'unsigned __int64 * {aka unsigned long long *}'.\n", errout.str());

        check("void foo() {\n"
              "    size_t s;\n"
              "    ptrdiff_t p;\n"
              "    __int32 i32;\n"
              "    unsigned __int32 u32;\n"
              "    __int64 i64;\n"
              "    unsigned __int64 u64;\n"
              "    scanf(\"%Id %Iu %Ix\", &s, &s, &s);\n"
              "    scanf(\"%Id %Iu %Ix\", &p, &p, &p);\n"
              "    scanf(\"%I32d %I32u %I32x\", &i32, &i32, &i32);\n"
              "    scanf(\"%I32d %I32u %I32x\", &u32, &u32, &u32);\n"
              "    scanf(\"%I64d %I64u %I64x\", &i64, &i64, &i64);\n"
              "    scanf(\"%I64d %I64u %I64x\", &u64, &u64, &u64);\n"
              "}", false, true, Settings::Win64);
        ASSERT_EQUALS("[test.cpp:8]: (portability) %Id in format string (no. 1) requires 'ptrdiff_t *' but the argument type is 'size_t * {aka unsigned long long *}'.\n"
                      "[test.cpp:9]: (portability) %Iu in format string (no. 2) requires 'size_t *' but the argument type is 'ptrdiff_t * {aka signed long long *}'.\n"
                      "[test.cpp:10]: (portability) %I32u in format string (no. 2) requires 'unsigned __int32 *' but the argument type is '__int32 * {aka signed int *}'.\n"
                      "[test.cpp:11]: (portability) %I32d in format string (no. 1) requires '__int32 *' but the argument type is 'unsigned __int32 * {aka unsigned int *}'.\n"
                      "[test.cpp:12]: (portability) %I64u in format string (no. 2) requires 'unsigned __int64 *' but the argument type is '__int64 * {aka signed long long *}'.\n"
                      "[test.cpp:13]: (portability) %I64d in format string (no. 1) requires '__int64 *' but the argument type is 'unsigned __int64 * {aka unsigned long long *}'.\n", errout.str());

        check("void foo() {\n"
              "    size_t s;\n"
              "    int i;\n"
              "    scanf(\"%I\", &s);\n"
              "    scanf(\"%I6\", &s);\n"
              "    scanf(\"%I6x\", &s);\n"
              "    scanf(\"%I16\", &s);\n"
              "    scanf(\"%I16x\", &s);\n"
              "    scanf(\"%I32\", &s);\n"
              "    scanf(\"%I64\", &s);\n"
              "    scanf(\"%I%i\", &s, &i);\n"
              "    scanf(\"%I6%i\", &s, &i);\n"
              "    scanf(\"%I6x%i\", &s, &i);\n"
              "    scanf(\"%I16%i\", &s, &i);\n"
              "    scanf(\"%I16x%i\", &s, &i);\n"
              "    scanf(\"%I32%i\", &s, &i);\n"
              "    scanf(\"%I64%i\", &s, &i);\n"
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

    void testMicrosoftCStringFormatArguments() { // ticket #4920
        check("void foo() {\n"
              "    unsigned __int32 u32;\n"
              "    String string;\n"
              "    string.Format(\"%I32d\", u32);\n"
              "    string.AppendFormat(\"%I32d\", u32);\n"
              "}", false, true, Settings::Win32A);
        ASSERT_EQUALS("", errout.str());

        check("void foo() {\n"
              "    unsigned __int32 u32;\n"
              "    CString string;\n"
              "    string.Format(\"%I32d\", u32);\n"
              "    string.AppendFormat(\"%I32d\", u32);\n"
              "}", false, true, Settings::Unix32);
        ASSERT_EQUALS("", errout.str());

        check("void foo() {\n"
              "    unsigned __int32 u32;\n"
              "    CString string;\n"
              "    string.Format(\"%I32d\", u32);\n"
              "    string.AppendFormat(\"%I32d\", u32);\n"
              "    CString::Format(\"%I32d\", u32);\n"
              "}", false, true, Settings::Win32A);
        ASSERT_EQUALS("[test.cpp:4]: (portability) %I32d in format string (no. 1) requires '__int32' but the argument type is 'unsigned __int32 {aka unsigned int}'.\n"
                      "[test.cpp:5]: (portability) %I32d in format string (no. 1) requires '__int32' but the argument type is 'unsigned __int32 {aka unsigned int}'.\n"
                      "[test.cpp:6]: (portability) %I32d in format string (no. 1) requires '__int32' but the argument type is 'unsigned __int32 {aka unsigned int}'.\n", errout.str());
    }

    void testMicrosoftSecurePrintfArgument() {
        check("void foo() {\n"
              "    int i;\n"
              "    unsigned int u;\n"
              "    _tprintf_s(_T(\"%d %u\"), u, i, 0);\n"
              "}\n", false, false, Settings::Win32A);
        ASSERT_EQUALS("[test.cpp:4]: (warning) %d in format string (no. 1) requires 'int' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:4]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'signed int'.\n"
                      "[test.cpp:4]: (warning) _tprintf_s format string requires 2 parameters but 3 are given.\n", errout.str());

        check("void foo() {\n"
              "    int i;\n"
              "    unsigned int u;\n"
              "    _tprintf_s(_T(\"%d %u\"), u, i, 0);\n"
              "}\n", false, false, Settings::Win32W);
        ASSERT_EQUALS("[test.cpp:4]: (warning) %d in format string (no. 1) requires 'int' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:4]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'signed int'.\n"
                      "[test.cpp:4]: (warning) _tprintf_s format string requires 2 parameters but 3 are given.\n", errout.str());

        check("void foo() {\n"
              "    int i;\n"
              "    unsigned int u;\n"
              "    printf_s(\"%d %u\", u, i, 0);\n"
              "}\n", false, false, Settings::Win32A);
        ASSERT_EQUALS("[test.cpp:4]: (warning) %d in format string (no. 1) requires 'int' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:4]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'signed int'.\n"
                      "[test.cpp:4]: (warning) printf_s format string requires 2 parameters but 3 are given.\n", errout.str());

        check("void foo() {\n"
              "    int i;\n"
              "    unsigned int u;\n"
              "    wprintf_s(L\"%d %u\", u, i, 0);\n"
              "}\n", false, false, Settings::Win32W);
        ASSERT_EQUALS("[test.cpp:4]: (warning) %d in format string (no. 1) requires 'int' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:4]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'signed int'.\n"
                      "[test.cpp:4]: (warning) wprintf_s format string requires 2 parameters but 3 are given.\n", errout.str());

        check("void foo() {\n"
              "    TCHAR str[10];\n"
              "    int i;\n"
              "    unsigned int u;\n"
              "    _stprintf_s(str, sizeof(str) / sizeof(TCHAR), _T(\"%d %u\"), u, i, 0);\n"
              "}\n", false, false, Settings::Win32A);
        ASSERT_EQUALS("[test.cpp:5]: (warning) %d in format string (no. 1) requires 'int' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:5]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'signed int'.\n"
                      "[test.cpp:5]: (warning) _stprintf_s format string requires 2 parameters but 3 are given.\n", errout.str());

        check("void foo() {\n"
              "    TCHAR str[10];\n"
              "    int i;\n"
              "    unsigned int u;\n"
              "    _stprintf_s(str, sizeof(str) / sizeof(TCHAR), _T(\"%d %u\"), u, i, 0);\n"
              "}\n", false, false, Settings::Win32W);
        ASSERT_EQUALS("[test.cpp:5]: (warning) %d in format string (no. 1) requires 'int' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:5]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'signed int'.\n"
                      "[test.cpp:5]: (warning) _stprintf_s format string requires 2 parameters but 3 are given.\n", errout.str());

        check("void foo() {\n"
              "    char str[10];\n"
              "    int i;\n"
              "    unsigned int u;\n"
              "    sprintf_s(str, sizeof(str), \"%d %u\", u, i, 0);\n"
              "}\n", false, false, Settings::Win32A);
        ASSERT_EQUALS("[test.cpp:5]: (warning) %d in format string (no. 1) requires 'int' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:5]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'signed int'.\n"
                      "[test.cpp:5]: (warning) sprintf_s format string requires 2 parameters but 3 are given.\n", errout.str());

        check("void foo() {\n"
              "    wchar_t str[10];\n"
              "    int i;\n"
              "    unsigned int u;\n"
              "    swprintf_s(str, sizeof(str) / sizeof(wchar_t), L\"%d %u\", u, i, 0);\n"
              "}\n", false, false, Settings::Win32W);
        ASSERT_EQUALS("[test.cpp:5]: (warning) %d in format string (no. 1) requires 'int' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:5]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'signed int'.\n"
                      "[test.cpp:5]: (warning) swprintf_s format string requires 2 parameters but 3 are given.\n", errout.str());

        check("void foo() {\n"
              "    TCHAR str[10];\n"
              "    int i;\n"
              "    unsigned int u;\n"
              "    _sntprintf_s(str, sizeof(str) / sizeof(TCHAR), _TRUNCATE, _T(\"%d %u\"), u, i, 0);\n"
              "}\n", false, false, Settings::Win32A);
        ASSERT_EQUALS("[test.cpp:5]: (warning) %d in format string (no. 1) requires 'int' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:5]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'signed int'.\n"
                      "[test.cpp:5]: (warning) _sntprintf_s format string requires 2 parameters but 3 are given.\n", errout.str());

        check("void foo() {\n"
              "    TCHAR str[10];\n"
              "    int i;\n"
              "    unsigned int u;\n"
              "    _sntprintf_s(str, sizeof(str) / sizeof(TCHAR), _TRUNCATE, _T(\"%d %u\"), u, i, 0);\n"
              "}\n", false, false, Settings::Win32W);
        ASSERT_EQUALS("[test.cpp:5]: (warning) %d in format string (no. 1) requires 'int' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:5]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'signed int'.\n"
                      "[test.cpp:5]: (warning) _sntprintf_s format string requires 2 parameters but 3 are given.\n", errout.str());

        check("void foo() {\n"
              "    char str[10];\n"
              "    int i;\n"
              "    unsigned int u;\n"
              "    _snprintf_s(str, sizeof(str), _TRUNCATE, \"%d %u\", u, i, 0);\n"
              "}\n", false, false, Settings::Win32A);
        ASSERT_EQUALS("[test.cpp:5]: (warning) %d in format string (no. 1) requires 'int' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:5]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'signed int'.\n"
                      "[test.cpp:5]: (warning) _snprintf_s format string requires 2 parameters but 3 are given.\n", errout.str());

        check("void foo() {\n"
              "    wchar_t str[10];\n"
              "    int i;\n"
              "    unsigned int u;\n"
              "    _snwprintf_s(str, sizeof(str) / sizeof(wchar_t), _TRUNCATE, L\"%d %u\", u, i, 0);\n"
              "}\n", false, false, Settings::Win32W);
        ASSERT_EQUALS("[test.cpp:5]: (warning) %d in format string (no. 1) requires 'int' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:5]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'signed int'.\n"
                      "[test.cpp:5]: (warning) _snwprintf_s format string requires 2 parameters but 3 are given.\n", errout.str());

        check("void foo(FILE * fp) {\n"
              "    int i;\n"
              "    unsigned int u;\n"
              "    _ftprintf_s(fp, _T(\"%d %u\"), u, i, 0);\n"
              "}\n", false, false, Settings::Win32A);
        ASSERT_EQUALS("[test.cpp:4]: (warning) %d in format string (no. 1) requires 'int' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:4]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'signed int'.\n"
                      "[test.cpp:4]: (warning) _ftprintf_s format string requires 2 parameters but 3 are given.\n", errout.str());

        check("void foo(FILE * fp) {\n"
              "    int i;\n"
              "    unsigned int u;\n"
              "    _ftprintf_s(fp, _T(\"%d %u\"), u, i, 0);\n"
              "}\n", false, false, Settings::Win32W);
        ASSERT_EQUALS("[test.cpp:4]: (warning) %d in format string (no. 1) requires 'int' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:4]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'signed int'.\n"
                      "[test.cpp:4]: (warning) _ftprintf_s format string requires 2 parameters but 3 are given.\n", errout.str());

        check("void foo(FILE * fp) {\n"
              "    int i;\n"
              "    unsigned int u;\n"
              "    fprintf_s(fp, \"%d %u\", u, i, 0);\n"
              "}\n", false, false, Settings::Win32A);
        ASSERT_EQUALS("[test.cpp:4]: (warning) %d in format string (no. 1) requires 'int' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:4]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'signed int'.\n"
                      "[test.cpp:4]: (warning) fprintf_s format string requires 2 parameters but 3 are given.\n", errout.str());

        check("void foo(FILE * fp) {\n"
              "    int i;\n"
              "    unsigned int u;\n"
              "    fwprintf_s(fp, L\"%d %u\", u, i, 0);\n"
              "}\n", false, false, Settings::Win32W);
        ASSERT_EQUALS("[test.cpp:4]: (warning) %d in format string (no. 1) requires 'int' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:4]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'signed int'.\n"
                      "[test.cpp:4]: (warning) fwprintf_s format string requires 2 parameters but 3 are given.\n", errout.str());

        check("void foo() {\n"
              "    char lineBuffer [600];\n"
              "    const char * const format = \"%15s%17s%17s%17s%17s\n\";\n"
              "    sprintf_s(lineBuffer, 600, format, \"type\", \"sum\", \"avg\", \"min\", \"max\");\n"
              "    sprintf_s(lineBuffer, format, \"type\", \"sum\", \"avg\", \"min\", \"max\");\n"
              "}\n", false, false, Settings::Win32A);
        ASSERT_EQUALS("", errout.str());

        check("void foo() {\n"
              "    const char * const format1 = \"%15s%17s%17s%17s%17s\n\";\n"
              "    const char format2[] = \"%15s%17s%17s%17s%17s\n\";\n"
              "    const char * const format3 = format1;\n"
              "    int i = 0;\n"
              "    sprintf_s(lineBuffer, format1, \"type\", \"sum\", \"avg\", \"min\", i, 0);\n"
              "    sprintf_s(lineBuffer, format2, \"type\", \"sum\", \"avg\", \"min\", i, 0);\n"
              "    sprintf_s(lineBuffer, format3, \"type\", \"sum\", \"avg\", \"min\", i, 0);\n"
              "    sprintf(lineBuffer, format1, \"type\", \"sum\", \"avg\", \"min\", i, 0);\n"
              "    sprintf(lineBuffer, format2, \"type\", \"sum\", \"avg\", \"min\", i, 0);\n"
              "    sprintf(lineBuffer, format3, \"type\", \"sum\", \"avg\", \"min\", i, 0);\n"
              "    printf(format1, \"type\", \"sum\", \"avg\", \"min\", i, 0);\n"
              "    printf(format2, \"type\", \"sum\", \"avg\", \"min\", i, 0);\n"
              "    printf(format3, \"type\", \"sum\", \"avg\", \"min\", i, 0);\n"
              "    sprintf_s(lineBuffer, 100, format1, \"type\", \"sum\", \"avg\", \"min\", i, 0);\n"
              "    sprintf_s(lineBuffer, 100, format2, \"type\", \"sum\", \"avg\", \"min\", i, 0);\n"
              "    sprintf_s(lineBuffer, 100, format3, \"type\", \"sum\", \"avg\", \"min\", i, 0);\n"
              "}\n", false, false, Settings::Win32A);
        ASSERT_EQUALS("[test.cpp:6]: (warning) %s in format string (no. 5) requires 'char *' but the argument type is 'signed int'.\n"
                      "[test.cpp:6]: (warning) sprintf_s format string requires 5 parameters but 6 are given.\n"
                      "[test.cpp:7]: (warning) %s in format string (no. 5) requires 'char *' but the argument type is 'signed int'.\n"
                      "[test.cpp:7]: (warning) sprintf_s format string requires 5 parameters but 6 are given.\n"
                      "[test.cpp:8]: (warning) %s in format string (no. 5) requires 'char *' but the argument type is 'signed int'.\n"
                      "[test.cpp:8]: (warning) sprintf_s format string requires 5 parameters but 6 are given.\n"
                      "[test.cpp:9]: (warning) %s in format string (no. 5) requires 'char *' but the argument type is 'signed int'.\n"
                      "[test.cpp:9]: (warning) sprintf format string requires 5 parameters but 6 are given.\n"
                      "[test.cpp:10]: (warning) %s in format string (no. 5) requires 'char *' but the argument type is 'signed int'.\n"
                      "[test.cpp:10]: (warning) sprintf format string requires 5 parameters but 6 are given.\n"
                      "[test.cpp:11]: (warning) %s in format string (no. 5) requires 'char *' but the argument type is 'signed int'.\n"
                      "[test.cpp:11]: (warning) sprintf format string requires 5 parameters but 6 are given.\n"
                      "[test.cpp:12]: (warning) %s in format string (no. 5) requires 'char *' but the argument type is 'signed int'.\n"
                      "[test.cpp:12]: (warning) printf format string requires 5 parameters but 6 are given.\n"
                      "[test.cpp:13]: (warning) %s in format string (no. 5) requires 'char *' but the argument type is 'signed int'.\n"
                      "[test.cpp:13]: (warning) printf format string requires 5 parameters but 6 are given.\n"
                      "[test.cpp:14]: (warning) %s in format string (no. 5) requires 'char *' but the argument type is 'signed int'.\n"
                      "[test.cpp:14]: (warning) printf format string requires 5 parameters but 6 are given.\n"
                      "[test.cpp:15]: (warning) %s in format string (no. 5) requires 'char *' but the argument type is 'signed int'.\n"
                      "[test.cpp:15]: (warning) sprintf_s format string requires 5 parameters but 6 are given.\n"
                      "[test.cpp:16]: (warning) %s in format string (no. 5) requires 'char *' but the argument type is 'signed int'.\n"
                      "[test.cpp:16]: (warning) sprintf_s format string requires 5 parameters but 6 are given.\n"
                      "[test.cpp:17]: (warning) %s in format string (no. 5) requires 'char *' but the argument type is 'signed int'.\n"
                      "[test.cpp:17]: (warning) sprintf_s format string requires 5 parameters but 6 are given.\n", errout.str());

    }

    void testMicrosoftSecureScanfArgument() {
        check("void foo() {\n"
              "    int i;\n"
              "    unsigned int u;\n"
              "    TCHAR str[10];\n"
              "    _tscanf_s(_T(\"%s %d %u %[a-z]\"), str, 10, &u, &i, str, 10, 0)\n"
              "}\n", false, false, Settings::Win32A);
        ASSERT_EQUALS("[test.cpp:5]: (warning) %d in format string (no. 2) requires 'int *' but the argument type is 'unsigned int *'.\n"
                      "[test.cpp:5]: (warning) %u in format string (no. 3) requires 'unsigned int *' but the argument type is 'signed int *'.\n"
                      "[test.cpp:5]: (warning) _tscanf_s format string requires 6 parameters but 7 are given.\n", errout.str());

        check("void foo() {\n"
              "    int i;\n"
              "    unsigned int u;\n"
              "    TCHAR str[10];\n"
              "    _tscanf_s(_T(\"%s %d %u %[a-z]\"), str, 10, &u, &i, str, 10, 0)\n"
              "}\n", false, false, Settings::Win32W);
        ASSERT_EQUALS("[test.cpp:5]: (warning) %d in format string (no. 2) requires 'int *' but the argument type is 'unsigned int *'.\n"
                      "[test.cpp:5]: (warning) %u in format string (no. 3) requires 'unsigned int *' but the argument type is 'signed int *'.\n"
                      "[test.cpp:5]: (warning) _tscanf_s format string requires 6 parameters but 7 are given.\n", errout.str());

        check("void foo() {\n"
              "    int i;\n"
              "    unsigned int u;\n"
              "    char str[10];\n"
              "    scanf_s(\"%s %d %u %[a-z]\", str, 10, &u, &i, str, 10, 0)\n"
              "}\n", false, false, Settings::Win32A);
        ASSERT_EQUALS("[test.cpp:5]: (warning) %d in format string (no. 2) requires 'int *' but the argument type is 'unsigned int *'.\n"
                      "[test.cpp:5]: (warning) %u in format string (no. 3) requires 'unsigned int *' but the argument type is 'signed int *'.\n"
                      "[test.cpp:5]: (warning) scanf_s format string requires 6 parameters but 7 are given.\n", errout.str());

        check("void foo() {\n"
              "    int i;\n"
              "    unsigned int u;\n"
              "    wchar_t str[10];\n"
              "    wscanf_s(L\"%s %d %u %[a-z]\", str, 10, &u, &i, str, 10, 0)\n"
              "}\n", false, false, Settings::Win32W);
        ASSERT_EQUALS("[test.cpp:5]: (warning) %d in format string (no. 2) requires 'int *' but the argument type is 'unsigned int *'.\n"
                      "[test.cpp:5]: (warning) %u in format string (no. 3) requires 'unsigned int *' but the argument type is 'signed int *'.\n"
                      "[test.cpp:5]: (warning) wscanf_s format string requires 6 parameters but 7 are given.\n", errout.str());

        check("void foo() {\n"
              "    TCHAR txt[100];\n"
              "    int i;\n"
              "    unsigned int u;\n"
              "    TCHAR str[10];\n"
              "    _stscanf_s(txt, _T(\"%s %d %u %[a-z]\"), str, 10, &u, &i, str, 10, 0)\n"
              "}\n", false, false, Settings::Win32A);
        ASSERT_EQUALS("[test.cpp:6]: (warning) %d in format string (no. 2) requires 'int *' but the argument type is 'unsigned int *'.\n"
                      "[test.cpp:6]: (warning) %u in format string (no. 3) requires 'unsigned int *' but the argument type is 'signed int *'.\n"
                      "[test.cpp:6]: (warning) _stscanf_s format string requires 6 parameters but 7 are given.\n", errout.str());

        check("void foo() {\n"
              "    TCHAR txt[100];\n"
              "    int i;\n"
              "    unsigned int u;\n"
              "    TCHAR str[10];\n"
              "    _stscanf_s(txt, _T(\"%s %d %u %[a-z]\"), str, 10, &u, &i, str, 10, 0)\n"
              "}\n", false, false, Settings::Win32W);
        ASSERT_EQUALS("[test.cpp:6]: (warning) %d in format string (no. 2) requires 'int *' but the argument type is 'unsigned int *'.\n"
                      "[test.cpp:6]: (warning) %u in format string (no. 3) requires 'unsigned int *' but the argument type is 'signed int *'.\n"
                      "[test.cpp:6]: (warning) _stscanf_s format string requires 6 parameters but 7 are given.\n", errout.str());

        check("void foo() {\n"
              "    char txt[100];\n"
              "    int i;\n"
              "    unsigned int u;\n"
              "    char str[10];\n"
              "    sscanf_s(txt, \"%s %d %u %[a-z]\", str, 10, &u, &i, str, 10, 0)\n"
              "}\n", false, false, Settings::Win32A);
        ASSERT_EQUALS("[test.cpp:6]: (warning) %d in format string (no. 2) requires 'int *' but the argument type is 'unsigned int *'.\n"
                      "[test.cpp:6]: (warning) %u in format string (no. 3) requires 'unsigned int *' but the argument type is 'signed int *'.\n"
                      "[test.cpp:6]: (warning) sscanf_s format string requires 6 parameters but 7 are given.\n", errout.str());

        check("void foo() {\n"
              "    wchar_t txt[100];\n"
              "    int i;\n"
              "    unsigned int u;\n"
              "    wchar_t str[10];\n"
              "    swscanf_s(txt, L\"%s %d %u %[a-z]\", str, 10, &u, &i, str, 10, 0)\n"
              "}\n", false, false, Settings::Win32W);
        ASSERT_EQUALS("[test.cpp:6]: (warning) %d in format string (no. 2) requires 'int *' but the argument type is 'unsigned int *'.\n"
                      "[test.cpp:6]: (warning) %u in format string (no. 3) requires 'unsigned int *' but the argument type is 'signed int *'.\n"
                      "[test.cpp:6]: (warning) swscanf_s format string requires 6 parameters but 7 are given.\n", errout.str());

        check("void foo(FILE * fp) {\n"
              "    int i;\n"
              "    unsigned int u;\n"
              "    TCHAR str[10];\n"
              "    _ftscanf_s(fp, _T(\"%s %d %u %[a-z]\"), str, 10, &u, &i, str, 10, 0)\n"
              "}\n", false, false, Settings::Win32A);
        ASSERT_EQUALS("[test.cpp:5]: (warning) %d in format string (no. 2) requires 'int *' but the argument type is 'unsigned int *'.\n"
                      "[test.cpp:5]: (warning) %u in format string (no. 3) requires 'unsigned int *' but the argument type is 'signed int *'.\n"
                      "[test.cpp:5]: (warning) _ftscanf_s format string requires 6 parameters but 7 are given.\n", errout.str());

        check("void foo(FILE * fp) {\n"
              "    int i;\n"
              "    unsigned int u;\n"
              "    TCHAR str[10];\n"
              "    _ftscanf_s(fp, _T(\"%s %d %u %[a-z]\"), str, 10, &u, &i, str, 10, 0)\n"
              "}\n", false, false, Settings::Win32W);
        ASSERT_EQUALS("[test.cpp:5]: (warning) %d in format string (no. 2) requires 'int *' but the argument type is 'unsigned int *'.\n"
                      "[test.cpp:5]: (warning) %u in format string (no. 3) requires 'unsigned int *' but the argument type is 'signed int *'.\n"
                      "[test.cpp:5]: (warning) _ftscanf_s format string requires 6 parameters but 7 are given.\n", errout.str());

        check("void foo(FILE * fp) {\n"
              "    int i;\n"
              "    unsigned int u;\n"
              "    char str[10];\n"
              "    fscanf_s(fp, \"%s %d %u %[a-z]\", str, 10, &u, &i, str, 10, 0)\n"
              "}\n", false, false, Settings::Win32A);
        ASSERT_EQUALS("[test.cpp:5]: (warning) %d in format string (no. 2) requires 'int *' but the argument type is 'unsigned int *'.\n"
                      "[test.cpp:5]: (warning) %u in format string (no. 3) requires 'unsigned int *' but the argument type is 'signed int *'.\n"
                      "[test.cpp:5]: (warning) fscanf_s format string requires 6 parameters but 7 are given.\n", errout.str());

        check("void foo(FILE * fp) {\n"
              "    int i;\n"
              "    unsigned int u;\n"
              "    wchar_t str[10];\n"
              "    fwscanf_s(fp, L\"%s %d %u %[a-z]\", str, 10, &u, &i, str, 10, 0)\n"
              "}\n", false, false, Settings::Win32W);
        ASSERT_EQUALS("[test.cpp:5]: (warning) %d in format string (no. 2) requires 'int *' but the argument type is 'unsigned int *'.\n"
                      "[test.cpp:5]: (warning) %u in format string (no. 3) requires 'unsigned int *' but the argument type is 'signed int *'.\n"
                      "[test.cpp:5]: (warning) fwscanf_s format string requires 6 parameters but 7 are given.\n", errout.str());

        check("void foo() {\n"
              "    WCHAR msStr1[5] = {0};\n"
              "    wscanf_s(L\"%4[^-]\", msStr1, _countof(msStr1));\n"
              "}\n", false, false, Settings::Win32W);
        ASSERT_EQUALS("", errout.str());
    }

    void testQStringFormatArguments() {
        check("void foo(float f) {\n"
              "    QString string;\n"
              "    string.sprintf(\"%d\", f);\n"
              "}", false, false, Settings::Win32A);
        ASSERT_EQUALS("[test.cpp:3]: (warning) %d in format string (no. 1) requires 'int' but the argument type is 'float'.\n", errout.str());

        check("void foo(float f) {\n"
              "    QString string;\n"
              "    string = QString::asprintf(\"%d\", f);\n"
              "}", false, false, Settings::Win32A);
        ASSERT_EQUALS("[test.cpp:3]: (warning) %d in format string (no. 1) requires 'int' but the argument type is 'float'.\n", errout.str());
    }

    void testTernary() {  // ticket #6182
        check("void test(const std::string &val) {\n"
              "    printf(\"%s\n\", val.empty() ? \"I like to eat bananas\" : val.c_str());\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void testUnsignedConst() {  // ticket #6321
        check("void test() {\n"
              "    unsigned const x = 5;\n"
              "    printf(\"%u\", x);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void testAstType() { // ticket #7014
        check("void test() {\n"
              "    printf(\"%c\", \"hello\"[0]);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void test() {\n"
              "    printf(\"%lld\", (long long)1);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());

        check("void test() {\n"
              "    printf(\"%i\", (short *)x);\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:2]: (warning) %i in format string (no. 1) requires 'int' but the argument type is 'signed short *'.\n", errout.str());

        check("int (*fp)();\n" // #7178 - function pointer call
              "void test() {\n"
              "    printf(\"%i\", fp());\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

    void testPrintf0WithSuffix() { // ticket #7069
        check("void foo() {\n"
              "    printf(\"%u %lu %llu\", 0U, 0UL, 0ULL);\n"
              "    printf(\"%u %lu %llu\", 0u, 0ul, 0ull);\n"
              "}\n");
        ASSERT_EQUALS("", errout.str());
    }

};

REGISTER_TEST(TestIO)
