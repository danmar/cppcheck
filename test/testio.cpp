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

        TEST_CASE(coutCerrMisusage);

        TEST_CASE(wrongMode_simple);
        TEST_CASE(wrongMode_complex);
        TEST_CASE(useClosedFile);
        TEST_CASE(fileIOwithoutPositioning);
        TEST_CASE(seekOnAppendedFile);
        TEST_CASE(fflushOnInputStream);

        TEST_CASE(testScanf1); // Scanf without field limiters
        TEST_CASE(testScanf2);
        TEST_CASE(testScanf3);
        TEST_CASE(testScanf4); // #ticket 2553

        TEST_CASE(testScanfArgument);
        TEST_CASE(testPrintfArgument);
        TEST_CASE(testPosixPrintfScanfParameterPosition);  // #4900

        TEST_CASE(testMicrosoftPrintfArgument); // ticket #4902
        TEST_CASE(testMicrosoftScanfArgument);
        TEST_CASE(testMicrosoftCStringFormatArguments); // ticket #4920
        TEST_CASE(testMicrosoftSecurePrintfArgument);
        TEST_CASE(testMicrosoftSecureScanfArgument);

        TEST_CASE(testTernary); // ticket #6182
        TEST_CASE(testUnsignedConst); // ticket #6132
    }

    void check(const char code[], bool inconclusive = false, bool portability = false, Settings::PlatformType platform = Settings::Unspecified) {
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
        ASSERT_EQUALS("[test.cpp:4]: (warning) scanf format string requires 0 parameters but 1 is given.\n", errout.str());
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

        {
            const char * code = "const unsigned int * foo() { }\n"
                                "void foo() {\n"
                                "    bool b;\n"
                                "    char c;\n"
                                "    signed char sc;\n"
                                "    unsigned char uc;\n"
                                "    short s;\n"
                                "    unsigned short us;\n"
                                "    int i;\n"
                                "    unsigned int ui;\n"
                                "    long l;\n"
                                "    unsigned long ul;\n"
                                "    long long ll;\n"
                                "    unsigned long long ull;\n"
                                "    float f;\n"
                                "    double d;\n"
                                "    long double ld;\n"
                                "    size_t st;\n"
                                "    ssize_t sst;\n"
                                "    ptrdiff_t pt;\n"
                                "    intmax_t it;\n"
                                "    uintmax_t ut;\n"
                                "    void * vp;\n"
                                "    std::size_t stdst;\n"
                                "    std::ssize_t stdsst;\n"
                                "    std::ptrdiff_t stdpt;\n"
                                "    std::intptr_t stdipt;\n"
                                "    std::uintptr_t stduipt;\n"
                                "    scanf(\"%u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u\",\n"
                                "          &b, &c, &sc, &uc, &s, &us, &i, &ui, &l, &ul, &ll, &ull, &f, &d, &ld, &st, &sst, &pt, &it, &ut, &vp, vp,\n"
                                "          \"str\", L\"str\", foo(), &unknown, unknown, &stdst, &stdsst, &stdpt, &stdipt, &stduipt);\n"
                                "}\n";
            std::string result("[test.cpp:29]: (warning) %u in format string (no. 1) requires 'unsigned int *' but the argument type is 'bool *'.\n"
                               "[test.cpp:29]: (warning) %u in format string (no. 2) requires 'unsigned int *' but the argument type is 'char *'.\n"
                               "[test.cpp:29]: (warning) %u in format string (no. 3) requires 'unsigned int *' but the argument type is 'signed char *'.\n"
                               "[test.cpp:29]: (warning) %u in format string (no. 4) requires 'unsigned int *' but the argument type is 'unsigned char *'.\n"
                               "[test.cpp:29]: (warning) %u in format string (no. 5) requires 'unsigned int *' but the argument type is 'short *'.\n"
                               "[test.cpp:29]: (warning) %u in format string (no. 6) requires 'unsigned int *' but the argument type is 'unsigned short *'.\n"
                               "[test.cpp:29]: (warning) %u in format string (no. 7) requires 'unsigned int *' but the argument type is 'int *'.\n"
                               "[test.cpp:29]: (warning) %u in format string (no. 9) requires 'unsigned int *' but the argument type is 'long *'.\n"
                               "[test.cpp:29]: (warning) %u in format string (no. 10) requires 'unsigned int *' but the argument type is 'unsigned long *'.\n"
                               "[test.cpp:29]: (warning) %u in format string (no. 11) requires 'unsigned int *' but the argument type is 'long long *'.\n"
                               "[test.cpp:29]: (warning) %u in format string (no. 12) requires 'unsigned int *' but the argument type is 'unsigned long long *'.\n"
                               "[test.cpp:29]: (warning) %u in format string (no. 13) requires 'unsigned int *' but the argument type is 'float *'.\n"
                               "[test.cpp:29]: (warning) %u in format string (no. 14) requires 'unsigned int *' but the argument type is 'double *'.\n"
                               "[test.cpp:29]: (warning) %u in format string (no. 15) requires 'unsigned int *' but the argument type is 'long double *'.\n"
                               "[test.cpp:29]: (warning) %u in format string (no. 16) requires 'unsigned int *' but the argument type is 'size_t * {aka unsigned long *}'.\n"
                               "[test.cpp:29]: (warning) %u in format string (no. 17) requires 'unsigned int *' but the argument type is 'ssize_t * {aka long *}'.\n"
                               "[test.cpp:29]: (warning) %u in format string (no. 18) requires 'unsigned int *' but the argument type is 'ptrdiff_t * {aka long *}'.\n"
                               "[test.cpp:29]: (warning) %u in format string (no. 19) requires 'unsigned int *' but the argument type is 'intmax_t * {aka long *}'.\n"
                               "[test.cpp:29]: (warning) %u in format string (no. 20) requires 'unsigned int *' but the argument type is 'uintmax_t * {aka unsigned long *}'.\n"
                               "[test.cpp:29]: (warning) %u in format string (no. 21) requires 'unsigned int *' but the argument type is 'void * *'.\n"
                               "[test.cpp:29]: (warning) %u in format string (no. 22) requires 'unsigned int *' but the argument type is 'void *'.\n"
                               "[test.cpp:29]: (warning) %u in format string (no. 23) requires 'unsigned int *' but the argument type is 'const char *'.\n"
                               "[test.cpp:29]: (warning) %u in format string (no. 24) requires 'unsigned int *' but the argument type is 'const wchar_t *'.\n"
                               "[test.cpp:29]: (warning) %u in format string (no. 25) requires 'unsigned int *' but the argument type is 'const unsigned int *'.\n"
                               "[test.cpp:29]: (warning) %u in format string (no. 28) requires 'unsigned int *' but the argument type is 'std::size_t * {aka unsigned long *}'.\n"
                               "[test.cpp:29]: (warning) %u in format string (no. 29) requires 'unsigned int *' but the argument type is 'std::ssize_t * {aka long *}'.\n"
                               "[test.cpp:29]: (warning) %u in format string (no. 30) requires 'unsigned int *' but the argument type is 'std::ptrdiff_t * {aka long *}'.\n"
                               "[test.cpp:29]: (warning) %u in format string (no. 31) requires 'unsigned int *' but the argument type is 'std::intptr_t * {aka long *}'.\n"
                               "[test.cpp:29]: (warning) %u in format string (no. 32) requires 'unsigned int *' but the argument type is 'std::uintptr_t * {aka unsigned long *}'.\n");
            std::string result_win64("[test.cpp:29]: (warning) %u in format string (no. 1) requires 'unsigned int *' but the argument type is 'bool *'.\n"
                                     "[test.cpp:29]: (warning) %u in format string (no. 2) requires 'unsigned int *' but the argument type is 'char *'.\n"
                                     "[test.cpp:29]: (warning) %u in format string (no. 3) requires 'unsigned int *' but the argument type is 'signed char *'.\n"
                                     "[test.cpp:29]: (warning) %u in format string (no. 4) requires 'unsigned int *' but the argument type is 'unsigned char *'.\n"
                                     "[test.cpp:29]: (warning) %u in format string (no. 5) requires 'unsigned int *' but the argument type is 'short *'.\n"
                                     "[test.cpp:29]: (warning) %u in format string (no. 6) requires 'unsigned int *' but the argument type is 'unsigned short *'.\n"
                                     "[test.cpp:29]: (warning) %u in format string (no. 7) requires 'unsigned int *' but the argument type is 'int *'.\n"
                                     "[test.cpp:29]: (warning) %u in format string (no. 9) requires 'unsigned int *' but the argument type is 'long *'.\n"
                                     "[test.cpp:29]: (warning) %u in format string (no. 10) requires 'unsigned int *' but the argument type is 'unsigned long *'.\n"
                                     "[test.cpp:29]: (warning) %u in format string (no. 11) requires 'unsigned int *' but the argument type is 'long long *'.\n"
                                     "[test.cpp:29]: (warning) %u in format string (no. 12) requires 'unsigned int *' but the argument type is 'unsigned long long *'.\n"
                                     "[test.cpp:29]: (warning) %u in format string (no. 13) requires 'unsigned int *' but the argument type is 'float *'.\n"
                                     "[test.cpp:29]: (warning) %u in format string (no. 14) requires 'unsigned int *' but the argument type is 'double *'.\n"
                                     "[test.cpp:29]: (warning) %u in format string (no. 15) requires 'unsigned int *' but the argument type is 'long double *'.\n"
                                     "[test.cpp:29]: (warning) %u in format string (no. 16) requires 'unsigned int *' but the argument type is 'size_t * {aka unsigned long long *}'.\n"
                                     "[test.cpp:29]: (warning) %u in format string (no. 17) requires 'unsigned int *' but the argument type is 'ssize_t * {aka long long *}'.\n"
                                     "[test.cpp:29]: (warning) %u in format string (no. 18) requires 'unsigned int *' but the argument type is 'ptrdiff_t * {aka long long *}'.\n"
                                     "[test.cpp:29]: (warning) %u in format string (no. 19) requires 'unsigned int *' but the argument type is 'intmax_t * {aka long long *}'.\n"
                                     "[test.cpp:29]: (warning) %u in format string (no. 20) requires 'unsigned int *' but the argument type is 'uintmax_t * {aka unsigned long long *}'.\n"
                                     "[test.cpp:29]: (warning) %u in format string (no. 21) requires 'unsigned int *' but the argument type is 'void * *'.\n"
                                     "[test.cpp:29]: (warning) %u in format string (no. 22) requires 'unsigned int *' but the argument type is 'void *'.\n"
                                     "[test.cpp:29]: (warning) %u in format string (no. 23) requires 'unsigned int *' but the argument type is 'const char *'.\n"
                                     "[test.cpp:29]: (warning) %u in format string (no. 24) requires 'unsigned int *' but the argument type is 'const wchar_t *'.\n"
                                     "[test.cpp:29]: (warning) %u in format string (no. 25) requires 'unsigned int *' but the argument type is 'const unsigned int *'.\n"
                                     "[test.cpp:29]: (warning) %u in format string (no. 28) requires 'unsigned int *' but the argument type is 'std::size_t * {aka unsigned long long *}'.\n"
                                     "[test.cpp:29]: (warning) %u in format string (no. 29) requires 'unsigned int *' but the argument type is 'std::ssize_t * {aka long long *}'.\n"
                                     "[test.cpp:29]: (warning) %u in format string (no. 30) requires 'unsigned int *' but the argument type is 'std::ptrdiff_t * {aka long long *}'.\n"
                                     "[test.cpp:29]: (warning) %u in format string (no. 31) requires 'unsigned int *' but the argument type is 'std::intptr_t * {aka long long *}'.\n"
                                     "[test.cpp:29]: (warning) %u in format string (no. 32) requires 'unsigned int *' but the argument type is 'std::uintptr_t * {aka unsigned long long *}'.\n");

            check(code, true, false, Settings::Unix32);
            ASSERT_EQUALS(result, errout.str());
            check(code, true, false, Settings::Unix64);
            ASSERT_EQUALS(result, errout.str());
            check(code, true, false, Settings::Win32A);
            ASSERT_EQUALS(result, errout.str());
            check(code, true, false, Settings::Win64);
            ASSERT_EQUALS(result_win64, errout.str());
        }

        {
            const char * code = "void foo() {\n"
                                "    bool b;\n"
                                "    char c;\n"
                                "    signed char sc;\n"
                                "    unsigned char uc;\n"
                                "    short s;\n"
                                "    unsigned short us;\n"
                                "    int i;\n"
                                "    unsigned int ui;\n"
                                "    long l;\n"
                                "    unsigned long ul;\n"
                                "    long long ll;\n"
                                "    unsigned long long ull;\n"
                                "    float f;\n"
                                "    double d;\n"
                                "    long double ld;\n"
                                "    size_t st;\n"
                                "    ssize_t sst;\n"
                                "    ptrdiff_t pt;\n"
                                "    intmax_t it;\n"
                                "    uintmax_t ut;\n"
                                "    void * vp;\n"
                                "    std::size_t stdst;\n"
                                "    std::ssize_t stdsst;\n"
                                "    std::ptrdiff_t stdpt;\n"
                                "    std::intptr_t stdipt;\n"
                                "    std::uintptr_t stduipt;\n"
                                "    scanf(\"%lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu\",\n"
                                "          &b, &c, &sc, &uc, &s, &us, &i, &ui, &l, &ul, &ll, &ull, &f, &d, &ld, &st, &sst, &pt, &it, &ut, &vp, vp,\n"
                                "          &unknown, unknown, &stdst, &stdsst, &stdpt, &stdipt, &stduipt);\n"
                                "}\n";
            std::string result("[test.cpp:28]: (warning) %lu in format string (no. 1) requires 'unsigned long *' but the argument type is 'bool *'.\n"
                               "[test.cpp:28]: (warning) %lu in format string (no. 2) requires 'unsigned long *' but the argument type is 'char *'.\n"
                               "[test.cpp:28]: (warning) %lu in format string (no. 3) requires 'unsigned long *' but the argument type is 'signed char *'.\n"
                               "[test.cpp:28]: (warning) %lu in format string (no. 4) requires 'unsigned long *' but the argument type is 'unsigned char *'.\n"
                               "[test.cpp:28]: (warning) %lu in format string (no. 5) requires 'unsigned long *' but the argument type is 'short *'.\n"
                               "[test.cpp:28]: (warning) %lu in format string (no. 6) requires 'unsigned long *' but the argument type is 'unsigned short *'.\n"
                               "[test.cpp:28]: (warning) %lu in format string (no. 7) requires 'unsigned long *' but the argument type is 'int *'.\n"
                               "[test.cpp:28]: (warning) %lu in format string (no. 8) requires 'unsigned long *' but the argument type is 'unsigned int *'.\n"
                               "[test.cpp:28]: (warning) %lu in format string (no. 9) requires 'unsigned long *' but the argument type is 'long *'.\n"
                               "[test.cpp:28]: (warning) %lu in format string (no. 11) requires 'unsigned long *' but the argument type is 'long long *'.\n"
                               "[test.cpp:28]: (warning) %lu in format string (no. 12) requires 'unsigned long *' but the argument type is 'unsigned long long *'.\n"
                               "[test.cpp:28]: (warning) %lu in format string (no. 13) requires 'unsigned long *' but the argument type is 'float *'.\n"
                               "[test.cpp:28]: (warning) %lu in format string (no. 14) requires 'unsigned long *' but the argument type is 'double *'.\n"
                               "[test.cpp:28]: (warning) %lu in format string (no. 15) requires 'unsigned long *' but the argument type is 'long double *'.\n"
                               "[test.cpp:28]: (warning) %lu in format string (no. 16) requires 'unsigned long *' but the argument type is 'size_t * {aka unsigned long *}'.\n"
                               "[test.cpp:28]: (warning) %lu in format string (no. 17) requires 'unsigned long *' but the argument type is 'ssize_t * {aka long *}'.\n"
                               "[test.cpp:28]: (warning) %lu in format string (no. 18) requires 'unsigned long *' but the argument type is 'ptrdiff_t * {aka long *}'.\n"
                               "[test.cpp:28]: (warning) %lu in format string (no. 19) requires 'unsigned long *' but the argument type is 'intmax_t * {aka long *}'.\n"
                               "[test.cpp:28]: (warning) %lu in format string (no. 20) requires 'unsigned long *' but the argument type is 'uintmax_t * {aka unsigned long *}'.\n"
                               "[test.cpp:28]: (warning) %lu in format string (no. 21) requires 'unsigned long *' but the argument type is 'void * *'.\n"
                               "[test.cpp:28]: (warning) %lu in format string (no. 22) requires 'unsigned long *' but the argument type is 'void *'.\n"
                               "[test.cpp:28]: (warning) %lu in format string (no. 25) requires 'unsigned long *' but the argument type is 'std::size_t * {aka unsigned long *}'.\n"
                               "[test.cpp:28]: (warning) %lu in format string (no. 26) requires 'unsigned long *' but the argument type is 'std::ssize_t * {aka long *}'.\n"
                               "[test.cpp:28]: (warning) %lu in format string (no. 27) requires 'unsigned long *' but the argument type is 'std::ptrdiff_t * {aka long *}'.\n"
                               "[test.cpp:28]: (warning) %lu in format string (no. 28) requires 'unsigned long *' but the argument type is 'std::intptr_t * {aka long *}'.\n");
            std::string result_win64("[test.cpp:28]: (warning) %lu in format string (no. 1) requires 'unsigned long *' but the argument type is 'bool *'.\n"
                                     "[test.cpp:28]: (warning) %lu in format string (no. 2) requires 'unsigned long *' but the argument type is 'char *'.\n"
                                     "[test.cpp:28]: (warning) %lu in format string (no. 3) requires 'unsigned long *' but the argument type is 'signed char *'.\n"
                                     "[test.cpp:28]: (warning) %lu in format string (no. 4) requires 'unsigned long *' but the argument type is 'unsigned char *'.\n"
                                     "[test.cpp:28]: (warning) %lu in format string (no. 5) requires 'unsigned long *' but the argument type is 'short *'.\n"
                                     "[test.cpp:28]: (warning) %lu in format string (no. 6) requires 'unsigned long *' but the argument type is 'unsigned short *'.\n"
                                     "[test.cpp:28]: (warning) %lu in format string (no. 7) requires 'unsigned long *' but the argument type is 'int *'.\n"
                                     "[test.cpp:28]: (warning) %lu in format string (no. 8) requires 'unsigned long *' but the argument type is 'unsigned int *'.\n"
                                     "[test.cpp:28]: (warning) %lu in format string (no. 9) requires 'unsigned long *' but the argument type is 'long *'.\n"
                                     "[test.cpp:28]: (warning) %lu in format string (no. 11) requires 'unsigned long *' but the argument type is 'long long *'.\n"
                                     "[test.cpp:28]: (warning) %lu in format string (no. 12) requires 'unsigned long *' but the argument type is 'unsigned long long *'.\n"
                                     "[test.cpp:28]: (warning) %lu in format string (no. 13) requires 'unsigned long *' but the argument type is 'float *'.\n"
                                     "[test.cpp:28]: (warning) %lu in format string (no. 14) requires 'unsigned long *' but the argument type is 'double *'.\n"
                                     "[test.cpp:28]: (warning) %lu in format string (no. 15) requires 'unsigned long *' but the argument type is 'long double *'.\n"
                                     "[test.cpp:28]: (warning) %lu in format string (no. 16) requires 'unsigned long *' but the argument type is 'size_t * {aka unsigned long long *}'.\n"
                                     "[test.cpp:28]: (warning) %lu in format string (no. 17) requires 'unsigned long *' but the argument type is 'ssize_t * {aka long long *}'.\n"
                                     "[test.cpp:28]: (warning) %lu in format string (no. 18) requires 'unsigned long *' but the argument type is 'ptrdiff_t * {aka long long *}'.\n"
                                     "[test.cpp:28]: (warning) %lu in format string (no. 19) requires 'unsigned long *' but the argument type is 'intmax_t * {aka long long *}'.\n"
                                     "[test.cpp:28]: (warning) %lu in format string (no. 20) requires 'unsigned long *' but the argument type is 'uintmax_t * {aka unsigned long long *}'.\n"
                                     "[test.cpp:28]: (warning) %lu in format string (no. 21) requires 'unsigned long *' but the argument type is 'void * *'.\n"
                                     "[test.cpp:28]: (warning) %lu in format string (no. 22) requires 'unsigned long *' but the argument type is 'void *'.\n"
                                     "[test.cpp:28]: (warning) %lu in format string (no. 25) requires 'unsigned long *' but the argument type is 'std::size_t * {aka unsigned long long *}'.\n"
                                     "[test.cpp:28]: (warning) %lu in format string (no. 26) requires 'unsigned long *' but the argument type is 'std::ssize_t * {aka long long *}'.\n"
                                     "[test.cpp:28]: (warning) %lu in format string (no. 27) requires 'unsigned long *' but the argument type is 'std::ptrdiff_t * {aka long long *}'.\n"
                                     "[test.cpp:28]: (warning) %lu in format string (no. 28) requires 'unsigned long *' but the argument type is 'std::intptr_t * {aka long long *}'.\n"
                                     "[test.cpp:28]: (warning) %lu in format string (no. 29) requires 'unsigned long *' but the argument type is 'std::uintptr_t * {aka unsigned long long *}'.\n");

            check(code, true, false, Settings::Unix32);
            ASSERT_EQUALS(result, errout.str());
            check(code, true, false, Settings::Unix64);
            ASSERT_EQUALS(result, errout.str());
            check(code, true, false, Settings::Win32A);
            ASSERT_EQUALS(result, errout.str());
            check(code, true, false, Settings::Win64);
            ASSERT_EQUALS(result_win64, errout.str());
        }

        {
            const char * code = "void foo() {\n"
                                "    bool b;\n"
                                "    char c;\n"
                                "    signed char sc;\n"
                                "    unsigned char uc;\n"
                                "    short s;\n"
                                "    unsigned short us;\n"
                                "    int i;\n"
                                "    unsigned int ui;\n"
                                "    long l;\n"
                                "    unsigned long ul;\n"
                                "    long long ll;\n"
                                "    unsigned long long ull;\n"
                                "    float f;\n"
                                "    double d;\n"
                                "    long double ld;\n"
                                "    size_t st;\n"
                                "    ssize_t sst;\n"
                                "    ptrdiff_t pt;\n"
                                "    intmax_t it;\n"
                                "    uintmax_t ut;\n"
                                "    void * vp;\n"
                                "    std::size_t stdst;\n"
                                "    std::ssize_t stdsst;\n"
                                "    std::ptrdiff_t stdpt;\n"
                                "    std::intptr_t stdipt;\n"
                                "    std::uintptr_t stduipt;\n"
                                "    scanf(\"%llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu\",\n"
                                "          &b, &c, &sc, &uc, &s, &us, &i, &ui, &l, &ul, &ll, &ull, &f, &d, &ld, &st, &sst, &pt, &it, &ut, &vp, vp,\n"
                                "          &unknown, unknown, &stdst, &stdsst, &stdpt, &stdipt, &stduipt);\n"
                                "}\n";
            std::string result("[test.cpp:28]: (warning) %llu in format string (no. 1) requires 'unsigned long long *' but the argument type is 'bool *'.\n"
                               "[test.cpp:28]: (warning) %llu in format string (no. 2) requires 'unsigned long long *' but the argument type is 'char *'.\n"
                               "[test.cpp:28]: (warning) %llu in format string (no. 3) requires 'unsigned long long *' but the argument type is 'signed char *'.\n"
                               "[test.cpp:28]: (warning) %llu in format string (no. 4) requires 'unsigned long long *' but the argument type is 'unsigned char *'.\n"
                               "[test.cpp:28]: (warning) %llu in format string (no. 5) requires 'unsigned long long *' but the argument type is 'short *'.\n"
                               "[test.cpp:28]: (warning) %llu in format string (no. 6) requires 'unsigned long long *' but the argument type is 'unsigned short *'.\n"
                               "[test.cpp:28]: (warning) %llu in format string (no. 7) requires 'unsigned long long *' but the argument type is 'int *'.\n"
                               "[test.cpp:28]: (warning) %llu in format string (no. 8) requires 'unsigned long long *' but the argument type is 'unsigned int *'.\n"
                               "[test.cpp:28]: (warning) %llu in format string (no. 9) requires 'unsigned long long *' but the argument type is 'long *'.\n"
                               "[test.cpp:28]: (warning) %llu in format string (no. 10) requires 'unsigned long long *' but the argument type is 'unsigned long *'.\n"
                               "[test.cpp:28]: (warning) %llu in format string (no. 11) requires 'unsigned long long *' but the argument type is 'long long *'.\n"
                               "[test.cpp:28]: (warning) %llu in format string (no. 13) requires 'unsigned long long *' but the argument type is 'float *'.\n"
                               "[test.cpp:28]: (warning) %llu in format string (no. 14) requires 'unsigned long long *' but the argument type is 'double *'.\n"
                               "[test.cpp:28]: (warning) %llu in format string (no. 15) requires 'unsigned long long *' but the argument type is 'long double *'.\n"
                               "[test.cpp:28]: (warning) %llu in format string (no. 16) requires 'unsigned long long *' but the argument type is 'size_t * {aka unsigned long *}'.\n"
                               "[test.cpp:28]: (warning) %llu in format string (no. 17) requires 'unsigned long long *' but the argument type is 'ssize_t * {aka long *}'.\n"
                               "[test.cpp:28]: (warning) %llu in format string (no. 18) requires 'unsigned long long *' but the argument type is 'ptrdiff_t * {aka long *}'.\n"
                               "[test.cpp:28]: (warning) %llu in format string (no. 19) requires 'unsigned long long *' but the argument type is 'intmax_t * {aka long *}'.\n"
                               "[test.cpp:28]: (warning) %llu in format string (no. 20) requires 'unsigned long long *' but the argument type is 'uintmax_t * {aka unsigned long *}'.\n"
                               "[test.cpp:28]: (warning) %llu in format string (no. 21) requires 'unsigned long long *' but the argument type is 'void * *'.\n"
                               "[test.cpp:28]: (warning) %llu in format string (no. 22) requires 'unsigned long long *' but the argument type is 'void *'.\n"
                               "[test.cpp:28]: (warning) %llu in format string (no. 25) requires 'unsigned long long *' but the argument type is 'std::size_t * {aka unsigned long *}'.\n"
                               "[test.cpp:28]: (warning) %llu in format string (no. 26) requires 'unsigned long long *' but the argument type is 'std::ssize_t * {aka long *}'.\n"
                               "[test.cpp:28]: (warning) %llu in format string (no. 27) requires 'unsigned long long *' but the argument type is 'std::ptrdiff_t * {aka long *}'.\n"
                               "[test.cpp:28]: (warning) %llu in format string (no. 28) requires 'unsigned long long *' but the argument type is 'std::intptr_t * {aka long *}'.\n"
                               "[test.cpp:28]: (warning) %llu in format string (no. 29) requires 'unsigned long long *' but the argument type is 'std::uintptr_t * {aka unsigned long *}'.\n");
            std::string result_win64("[test.cpp:28]: (warning) %llu in format string (no. 1) requires 'unsigned long long *' but the argument type is 'bool *'.\n"
                                     "[test.cpp:28]: (warning) %llu in format string (no. 2) requires 'unsigned long long *' but the argument type is 'char *'.\n"
                                     "[test.cpp:28]: (warning) %llu in format string (no. 3) requires 'unsigned long long *' but the argument type is 'signed char *'.\n"
                                     "[test.cpp:28]: (warning) %llu in format string (no. 4) requires 'unsigned long long *' but the argument type is 'unsigned char *'.\n"
                                     "[test.cpp:28]: (warning) %llu in format string (no. 5) requires 'unsigned long long *' but the argument type is 'short *'.\n"
                                     "[test.cpp:28]: (warning) %llu in format string (no. 6) requires 'unsigned long long *' but the argument type is 'unsigned short *'.\n"
                                     "[test.cpp:28]: (warning) %llu in format string (no. 7) requires 'unsigned long long *' but the argument type is 'int *'.\n"
                                     "[test.cpp:28]: (warning) %llu in format string (no. 8) requires 'unsigned long long *' but the argument type is 'unsigned int *'.\n"
                                     "[test.cpp:28]: (warning) %llu in format string (no. 9) requires 'unsigned long long *' but the argument type is 'long *'.\n"
                                     "[test.cpp:28]: (warning) %llu in format string (no. 10) requires 'unsigned long long *' but the argument type is 'unsigned long *'.\n"
                                     "[test.cpp:28]: (warning) %llu in format string (no. 11) requires 'unsigned long long *' but the argument type is 'long long *'.\n"
                                     "[test.cpp:28]: (warning) %llu in format string (no. 13) requires 'unsigned long long *' but the argument type is 'float *'.\n"
                                     "[test.cpp:28]: (warning) %llu in format string (no. 14) requires 'unsigned long long *' but the argument type is 'double *'.\n"
                                     "[test.cpp:28]: (warning) %llu in format string (no. 15) requires 'unsigned long long *' but the argument type is 'long double *'.\n"
                                     "[test.cpp:28]: (warning) %llu in format string (no. 16) requires 'unsigned long long *' but the argument type is 'size_t * {aka unsigned long long *}'.\n"
                                     "[test.cpp:28]: (warning) %llu in format string (no. 17) requires 'unsigned long long *' but the argument type is 'ssize_t * {aka long long *}'.\n"
                                     "[test.cpp:28]: (warning) %llu in format string (no. 18) requires 'unsigned long long *' but the argument type is 'ptrdiff_t * {aka long long *}'.\n"
                                     "[test.cpp:28]: (warning) %llu in format string (no. 19) requires 'unsigned long long *' but the argument type is 'intmax_t * {aka long long *}'.\n"
                                     "[test.cpp:28]: (warning) %llu in format string (no. 20) requires 'unsigned long long *' but the argument type is 'uintmax_t * {aka unsigned long long *}'.\n"
                                     "[test.cpp:28]: (warning) %llu in format string (no. 21) requires 'unsigned long long *' but the argument type is 'void * *'.\n"
                                     "[test.cpp:28]: (warning) %llu in format string (no. 22) requires 'unsigned long long *' but the argument type is 'void *'.\n"
                                     "[test.cpp:28]: (warning) %llu in format string (no. 25) requires 'unsigned long long *' but the argument type is 'std::size_t * {aka unsigned long long *}'.\n"
                                     "[test.cpp:28]: (warning) %llu in format string (no. 26) requires 'unsigned long long *' but the argument type is 'std::ssize_t * {aka long long *}'.\n"
                                     "[test.cpp:28]: (warning) %llu in format string (no. 27) requires 'unsigned long long *' but the argument type is 'std::ptrdiff_t * {aka long long *}'.\n"
                                     "[test.cpp:28]: (warning) %llu in format string (no. 28) requires 'unsigned long long *' but the argument type is 'std::intptr_t * {aka long long *}'.\n");

            check(code, true, false, Settings::Unix32);
            ASSERT_EQUALS(result, errout.str());
            check(code, true, false, Settings::Unix64);
            ASSERT_EQUALS(result, errout.str());
            check(code, true, false, Settings::Win32A);
            ASSERT_EQUALS(result, errout.str());
            check(code, true, false, Settings::Win64);
            ASSERT_EQUALS(result_win64, errout.str());
        }

        {
            const char * code = "void foo() {\n"
                                "    bool b;\n"
                                "    char c;\n"
                                "    signed char sc;\n"
                                "    unsigned char uc;\n"
                                "    short s;\n"
                                "    unsigned short us;\n"
                                "    int i;\n"
                                "    unsigned int ui;\n"
                                "    long l;\n"
                                "    unsigned long ul;\n"
                                "    long long ll;\n"
                                "    unsigned long long ull;\n"
                                "    float f;\n"
                                "    double d;\n"
                                "    long double ld;\n"
                                "    size_t st;\n"
                                "    ssize_t sst;\n"
                                "    ptrdiff_t pt;\n"
                                "    intmax_t it;\n"
                                "    uintmax_t ut;\n"
                                "    void * vp;\n"
                                "    std::size_t stdst;\n"
                                "    std::ssize_t stdsst;\n"
                                "    std::ptrdiff_t stdpt;\n"
                                "    std::intptr_t stdipt;\n"
                                "    std::uintptr_t stduipt;\n"
                                "    scanf(\"%hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu\",\n"
                                "          &b, &c, &sc, &uc, &s, &us, &i, &ui, &l, &ul, &ll, &ull, &f, &d, &ld, &st, &sst, &pt, &it, &ut, &vp, vp,\n"
                                "          &unknown, unknown, &stdst, &stdsst, &stdpt, &stdipt, &stduipt);\n"
                                "}\n";
            std::string result("[test.cpp:28]: (warning) %hu in format string (no. 1) requires 'unsigned short *' but the argument type is 'bool *'.\n"
                               "[test.cpp:28]: (warning) %hu in format string (no. 2) requires 'unsigned short *' but the argument type is 'char *'.\n"
                               "[test.cpp:28]: (warning) %hu in format string (no. 3) requires 'unsigned short *' but the argument type is 'signed char *'.\n"
                               "[test.cpp:28]: (warning) %hu in format string (no. 4) requires 'unsigned short *' but the argument type is 'unsigned char *'.\n"
                               "[test.cpp:28]: (warning) %hu in format string (no. 5) requires 'unsigned short *' but the argument type is 'short *'.\n"
                               "[test.cpp:28]: (warning) %hu in format string (no. 7) requires 'unsigned short *' but the argument type is 'int *'.\n"
                               "[test.cpp:28]: (warning) %hu in format string (no. 8) requires 'unsigned short *' but the argument type is 'unsigned int *'.\n"
                               "[test.cpp:28]: (warning) %hu in format string (no. 9) requires 'unsigned short *' but the argument type is 'long *'.\n"
                               "[test.cpp:28]: (warning) %hu in format string (no. 10) requires 'unsigned short *' but the argument type is 'unsigned long *'.\n"
                               "[test.cpp:28]: (warning) %hu in format string (no. 11) requires 'unsigned short *' but the argument type is 'long long *'.\n"
                               "[test.cpp:28]: (warning) %hu in format string (no. 12) requires 'unsigned short *' but the argument type is 'unsigned long long *'.\n"
                               "[test.cpp:28]: (warning) %hu in format string (no. 13) requires 'unsigned short *' but the argument type is 'float *'.\n"
                               "[test.cpp:28]: (warning) %hu in format string (no. 14) requires 'unsigned short *' but the argument type is 'double *'.\n"
                               "[test.cpp:28]: (warning) %hu in format string (no. 15) requires 'unsigned short *' but the argument type is 'long double *'.\n"
                               "[test.cpp:28]: (warning) %hu in format string (no. 16) requires 'unsigned short *' but the argument type is 'size_t * {aka unsigned long *}'.\n"
                               "[test.cpp:28]: (warning) %hu in format string (no. 17) requires 'unsigned short *' but the argument type is 'ssize_t * {aka long *}'.\n"
                               "[test.cpp:28]: (warning) %hu in format string (no. 18) requires 'unsigned short *' but the argument type is 'ptrdiff_t * {aka long *}'.\n"
                               "[test.cpp:28]: (warning) %hu in format string (no. 19) requires 'unsigned short *' but the argument type is 'intmax_t * {aka long *}'.\n"
                               "[test.cpp:28]: (warning) %hu in format string (no. 20) requires 'unsigned short *' but the argument type is 'uintmax_t * {aka unsigned long *}'.\n"
                               "[test.cpp:28]: (warning) %hu in format string (no. 21) requires 'unsigned short *' but the argument type is 'void * *'.\n"
                               "[test.cpp:28]: (warning) %hu in format string (no. 22) requires 'unsigned short *' but the argument type is 'void *'.\n"
                               "[test.cpp:28]: (warning) %hu in format string (no. 25) requires 'unsigned short *' but the argument type is 'std::size_t * {aka unsigned long *}'.\n"
                               "[test.cpp:28]: (warning) %hu in format string (no. 26) requires 'unsigned short *' but the argument type is 'std::ssize_t * {aka long *}'.\n"
                               "[test.cpp:28]: (warning) %hu in format string (no. 27) requires 'unsigned short *' but the argument type is 'std::ptrdiff_t * {aka long *}'.\n"
                               "[test.cpp:28]: (warning) %hu in format string (no. 28) requires 'unsigned short *' but the argument type is 'std::intptr_t * {aka long *}'.\n"
                               "[test.cpp:28]: (warning) %hu in format string (no. 29) requires 'unsigned short *' but the argument type is 'std::uintptr_t * {aka unsigned long *}'.\n");
            std::string result_win64("[test.cpp:28]: (warning) %hu in format string (no. 1) requires 'unsigned short *' but the argument type is 'bool *'.\n"
                                     "[test.cpp:28]: (warning) %hu in format string (no. 2) requires 'unsigned short *' but the argument type is 'char *'.\n"
                                     "[test.cpp:28]: (warning) %hu in format string (no. 3) requires 'unsigned short *' but the argument type is 'signed char *'.\n"
                                     "[test.cpp:28]: (warning) %hu in format string (no. 4) requires 'unsigned short *' but the argument type is 'unsigned char *'.\n"
                                     "[test.cpp:28]: (warning) %hu in format string (no. 5) requires 'unsigned short *' but the argument type is 'short *'.\n"
                                     "[test.cpp:28]: (warning) %hu in format string (no. 7) requires 'unsigned short *' but the argument type is 'int *'.\n"
                                     "[test.cpp:28]: (warning) %hu in format string (no. 8) requires 'unsigned short *' but the argument type is 'unsigned int *'.\n"
                                     "[test.cpp:28]: (warning) %hu in format string (no. 9) requires 'unsigned short *' but the argument type is 'long *'.\n"
                                     "[test.cpp:28]: (warning) %hu in format string (no. 10) requires 'unsigned short *' but the argument type is 'unsigned long *'.\n"
                                     "[test.cpp:28]: (warning) %hu in format string (no. 11) requires 'unsigned short *' but the argument type is 'long long *'.\n"
                                     "[test.cpp:28]: (warning) %hu in format string (no. 12) requires 'unsigned short *' but the argument type is 'unsigned long long *'.\n"
                                     "[test.cpp:28]: (warning) %hu in format string (no. 13) requires 'unsigned short *' but the argument type is 'float *'.\n"
                                     "[test.cpp:28]: (warning) %hu in format string (no. 14) requires 'unsigned short *' but the argument type is 'double *'.\n"
                                     "[test.cpp:28]: (warning) %hu in format string (no. 15) requires 'unsigned short *' but the argument type is 'long double *'.\n"
                                     "[test.cpp:28]: (warning) %hu in format string (no. 16) requires 'unsigned short *' but the argument type is 'size_t * {aka unsigned long long *}'.\n"
                                     "[test.cpp:28]: (warning) %hu in format string (no. 17) requires 'unsigned short *' but the argument type is 'ssize_t * {aka long long *}'.\n"
                                     "[test.cpp:28]: (warning) %hu in format string (no. 18) requires 'unsigned short *' but the argument type is 'ptrdiff_t * {aka long long *}'.\n"
                                     "[test.cpp:28]: (warning) %hu in format string (no. 19) requires 'unsigned short *' but the argument type is 'intmax_t * {aka long long *}'.\n"
                                     "[test.cpp:28]: (warning) %hu in format string (no. 20) requires 'unsigned short *' but the argument type is 'uintmax_t * {aka unsigned long long *}'.\n"
                                     "[test.cpp:28]: (warning) %hu in format string (no. 21) requires 'unsigned short *' but the argument type is 'void * *'.\n"
                                     "[test.cpp:28]: (warning) %hu in format string (no. 22) requires 'unsigned short *' but the argument type is 'void *'.\n"
                                     "[test.cpp:28]: (warning) %hu in format string (no. 25) requires 'unsigned short *' but the argument type is 'std::size_t * {aka unsigned long long *}'.\n"
                                     "[test.cpp:28]: (warning) %hu in format string (no. 26) requires 'unsigned short *' but the argument type is 'std::ssize_t * {aka long long *}'.\n"
                                     "[test.cpp:28]: (warning) %hu in format string (no. 27) requires 'unsigned short *' but the argument type is 'std::ptrdiff_t * {aka long long *}'.\n"
                                     "[test.cpp:28]: (warning) %hu in format string (no. 28) requires 'unsigned short *' but the argument type is 'std::intptr_t * {aka long long *}'.\n"
                                     "[test.cpp:28]: (warning) %hu in format string (no. 29) requires 'unsigned short *' but the argument type is 'std::uintptr_t * {aka unsigned long long *}'.\n");

            check(code, true, false, Settings::Unix32);
            ASSERT_EQUALS(result, errout.str());
            check(code, true, false, Settings::Unix64);
            ASSERT_EQUALS(result, errout.str());
            check(code, true, false, Settings::Win32A);
            ASSERT_EQUALS(result, errout.str());
            check(code, true, false, Settings::Win64);
            ASSERT_EQUALS(result_win64, errout.str());
        }

        {
            const char * code = "void foo() {\n"
                                "    bool b;\n"
                                "    char c;\n"
                                "    signed char sc;\n"
                                "    unsigned char uc;\n"
                                "    short s;\n"
                                "    unsigned short us;\n"
                                "    int i;\n"
                                "    unsigned int ui;\n"
                                "    long l;\n"
                                "    unsigned long ul;\n"
                                "    long long ll;\n"
                                "    unsigned long long ull;\n"
                                "    float f;\n"
                                "    double d;\n"
                                "    long double ld;\n"
                                "    size_t st;\n"
                                "    ssize_t sst;\n"
                                "    ptrdiff_t pt;\n"
                                "    intmax_t it;\n"
                                "    uintmax_t ut;\n"
                                "    void * vp;\n"
                                "    std::size_t stdst;\n"
                                "    std::ssize_t stdsst;\n"
                                "    std::ptrdiff_t stdpt;\n"
                                "    std::intptr_t stdipt;\n"
                                "    std::uintptr_t stduipt;\n"
                                "    scanf(\"%hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu\",\n"
                                "          &b, &c, &sc, &uc, &s, &us, &i, &ui, &l, &ul, &ll, &ull, &f, &d, &ld, &st, &sst, &pt, &it, &ut, &vp, vp,\n"
                                "          &unknown, unknown, &stdst, &stdsst, &stdpt, &stdipt, &stduipt);\n"
                                "}\n";
            std::string result("[test.cpp:28]: (warning) %hhu in format string (no. 1) requires 'unsigned char *' but the argument type is 'bool *'.\n"
                               "[test.cpp:28]: (warning) %hhu in format string (no. 2) requires 'unsigned char *' but the argument type is 'char *'.\n"
                               "[test.cpp:28]: (warning) %hhu in format string (no. 3) requires 'unsigned char *' but the argument type is 'signed char *'.\n"
                               "[test.cpp:28]: (warning) %hhu in format string (no. 5) requires 'unsigned char *' but the argument type is 'short *'.\n"
                               "[test.cpp:28]: (warning) %hhu in format string (no. 6) requires 'unsigned char *' but the argument type is 'unsigned short *'.\n"
                               "[test.cpp:28]: (warning) %hhu in format string (no. 7) requires 'unsigned char *' but the argument type is 'int *'.\n"
                               "[test.cpp:28]: (warning) %hhu in format string (no. 8) requires 'unsigned char *' but the argument type is 'unsigned int *'.\n"
                               "[test.cpp:28]: (warning) %hhu in format string (no. 9) requires 'unsigned char *' but the argument type is 'long *'.\n"
                               "[test.cpp:28]: (warning) %hhu in format string (no. 10) requires 'unsigned char *' but the argument type is 'unsigned long *'.\n"
                               "[test.cpp:28]: (warning) %hhu in format string (no. 11) requires 'unsigned char *' but the argument type is 'long long *'.\n"
                               "[test.cpp:28]: (warning) %hhu in format string (no. 12) requires 'unsigned char *' but the argument type is 'unsigned long long *'.\n"
                               "[test.cpp:28]: (warning) %hhu in format string (no. 13) requires 'unsigned char *' but the argument type is 'float *'.\n"
                               "[test.cpp:28]: (warning) %hhu in format string (no. 14) requires 'unsigned char *' but the argument type is 'double *'.\n"
                               "[test.cpp:28]: (warning) %hhu in format string (no. 15) requires 'unsigned char *' but the argument type is 'long double *'.\n"
                               "[test.cpp:28]: (warning) %hhu in format string (no. 16) requires 'unsigned char *' but the argument type is 'size_t * {aka unsigned long *}'.\n"
                               "[test.cpp:28]: (warning) %hhu in format string (no. 17) requires 'unsigned char *' but the argument type is 'ssize_t * {aka long *}'.\n"
                               "[test.cpp:28]: (warning) %hhu in format string (no. 18) requires 'unsigned char *' but the argument type is 'ptrdiff_t * {aka long *}'.\n"
                               "[test.cpp:28]: (warning) %hhu in format string (no. 19) requires 'unsigned char *' but the argument type is 'intmax_t * {aka long *}'.\n"
                               "[test.cpp:28]: (warning) %hhu in format string (no. 20) requires 'unsigned char *' but the argument type is 'uintmax_t * {aka unsigned long *}'.\n"
                               "[test.cpp:28]: (warning) %hhu in format string (no. 21) requires 'unsigned char *' but the argument type is 'void * *'.\n"
                               "[test.cpp:28]: (warning) %hhu in format string (no. 22) requires 'unsigned char *' but the argument type is 'void *'.\n"
                               "[test.cpp:28]: (warning) %hhu in format string (no. 25) requires 'unsigned char *' but the argument type is 'std::size_t * {aka unsigned long *}'.\n"
                               "[test.cpp:28]: (warning) %hhu in format string (no. 26) requires 'unsigned char *' but the argument type is 'std::ssize_t * {aka long *}'.\n"
                               "[test.cpp:28]: (warning) %hhu in format string (no. 27) requires 'unsigned char *' but the argument type is 'std::ptrdiff_t * {aka long *}'.\n"
                               "[test.cpp:28]: (warning) %hhu in format string (no. 28) requires 'unsigned char *' but the argument type is 'std::intptr_t * {aka long *}'.\n"
                               "[test.cpp:28]: (warning) %hhu in format string (no. 29) requires 'unsigned char *' but the argument type is 'std::uintptr_t * {aka unsigned long *}'.\n");
            std::string result_win64("[test.cpp:28]: (warning) %hhu in format string (no. 1) requires 'unsigned char *' but the argument type is 'bool *'.\n"
                                     "[test.cpp:28]: (warning) %hhu in format string (no. 2) requires 'unsigned char *' but the argument type is 'char *'.\n"
                                     "[test.cpp:28]: (warning) %hhu in format string (no. 3) requires 'unsigned char *' but the argument type is 'signed char *'.\n"
                                     "[test.cpp:28]: (warning) %hhu in format string (no. 5) requires 'unsigned char *' but the argument type is 'short *'.\n"
                                     "[test.cpp:28]: (warning) %hhu in format string (no. 6) requires 'unsigned char *' but the argument type is 'unsigned short *'.\n"
                                     "[test.cpp:28]: (warning) %hhu in format string (no. 7) requires 'unsigned char *' but the argument type is 'int *'.\n"
                                     "[test.cpp:28]: (warning) %hhu in format string (no. 8) requires 'unsigned char *' but the argument type is 'unsigned int *'.\n"
                                     "[test.cpp:28]: (warning) %hhu in format string (no. 9) requires 'unsigned char *' but the argument type is 'long *'.\n"
                                     "[test.cpp:28]: (warning) %hhu in format string (no. 10) requires 'unsigned char *' but the argument type is 'unsigned long *'.\n"
                                     "[test.cpp:28]: (warning) %hhu in format string (no. 11) requires 'unsigned char *' but the argument type is 'long long *'.\n"
                                     "[test.cpp:28]: (warning) %hhu in format string (no. 12) requires 'unsigned char *' but the argument type is 'unsigned long long *'.\n"
                                     "[test.cpp:28]: (warning) %hhu in format string (no. 13) requires 'unsigned char *' but the argument type is 'float *'.\n"
                                     "[test.cpp:28]: (warning) %hhu in format string (no. 14) requires 'unsigned char *' but the argument type is 'double *'.\n"
                                     "[test.cpp:28]: (warning) %hhu in format string (no. 15) requires 'unsigned char *' but the argument type is 'long double *'.\n"
                                     "[test.cpp:28]: (warning) %hhu in format string (no. 16) requires 'unsigned char *' but the argument type is 'size_t * {aka unsigned long long *}'.\n"
                                     "[test.cpp:28]: (warning) %hhu in format string (no. 17) requires 'unsigned char *' but the argument type is 'ssize_t * {aka long long *}'.\n"
                                     "[test.cpp:28]: (warning) %hhu in format string (no. 18) requires 'unsigned char *' but the argument type is 'ptrdiff_t * {aka long long *}'.\n"
                                     "[test.cpp:28]: (warning) %hhu in format string (no. 19) requires 'unsigned char *' but the argument type is 'intmax_t * {aka long long *}'.\n"
                                     "[test.cpp:28]: (warning) %hhu in format string (no. 20) requires 'unsigned char *' but the argument type is 'uintmax_t * {aka unsigned long long *}'.\n"
                                     "[test.cpp:28]: (warning) %hhu in format string (no. 21) requires 'unsigned char *' but the argument type is 'void * *'.\n"
                                     "[test.cpp:28]: (warning) %hhu in format string (no. 22) requires 'unsigned char *' but the argument type is 'void *'.\n"
                                     "[test.cpp:28]: (warning) %hhu in format string (no. 25) requires 'unsigned char *' but the argument type is 'std::size_t * {aka unsigned long long *}'.\n"
                                     "[test.cpp:28]: (warning) %hhu in format string (no. 26) requires 'unsigned char *' but the argument type is 'std::ssize_t * {aka long long *}'.\n"
                                     "[test.cpp:28]: (warning) %hhu in format string (no. 27) requires 'unsigned char *' but the argument type is 'std::ptrdiff_t * {aka long long *}'.\n"
                                     "[test.cpp:28]: (warning) %hhu in format string (no. 28) requires 'unsigned char *' but the argument type is 'std::intptr_t * {aka long long *}'.\n"
                                     "[test.cpp:28]: (warning) %hhu in format string (no. 29) requires 'unsigned char *' but the argument type is 'std::uintptr_t * {aka unsigned long long *}'.\n");

            check(code, true, false, Settings::Unix32);
            ASSERT_EQUALS(result, errout.str());
            check(code, true, false, Settings::Unix64);
            ASSERT_EQUALS(result, errout.str());
            check(code, true, false, Settings::Win32A);
            ASSERT_EQUALS(result, errout.str());
            check(code, true, false, Settings::Win64);
            ASSERT_EQUALS(result_win64, errout.str());
        }

        {
            const char * code = "void foo() {\n"
                                "    bool b;\n"
                                "    char c;\n"
                                "    signed char sc;\n"
                                "    unsigned char uc;\n"
                                "    short s;\n"
                                "    unsigned short us;\n"
                                "    int i;\n"
                                "    unsigned int ui;\n"
                                "    long l;\n"
                                "    unsigned long ul;\n"
                                "    long long ll;\n"
                                "    unsigned long long ull;\n"
                                "    float f;\n"
                                "    double d;\n"
                                "    long double ld;\n"
                                "    size_t st;\n"
                                "    ssize_t sst;\n"
                                "    ptrdiff_t pt;\n"
                                "    intmax_t it;\n"
                                "    uintmax_t ut;\n"
                                "    void * vp;\n"
                                "    std::size_t stdst;\n"
                                "    std::ssize_t stdsst;\n"
                                "    std::ptrdiff_t stdpt;\n"
                                "    std::intptr_t stdipt;\n"
                                "    std::uintptr_t stduipt;\n"
                                "    scanf(\"%Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu\",\n"
                                "          &b, &c, &sc, &uc, &s, &us, &i, &ui, &l, &ul, &ll, &ull, &f, &d, &ld, &st, &sst, &pt, &it, &ut, &vp, vp,\n"
                                "          &unknown, unknown, &stdst, &stdsst, &stdpt, &stdipt, &stduipt);\n"
                                "}\n";
            std::string result("[test.cpp:28]: (warning) %Lu in format string (no. 1) requires 'unsigned long long *' but the argument type is 'bool *'.\n"
                               "[test.cpp:28]: (warning) %Lu in format string (no. 2) requires 'unsigned long long *' but the argument type is 'char *'.\n"
                               "[test.cpp:28]: (warning) %Lu in format string (no. 3) requires 'unsigned long long *' but the argument type is 'signed char *'.\n"
                               "[test.cpp:28]: (warning) %Lu in format string (no. 4) requires 'unsigned long long *' but the argument type is 'unsigned char *'.\n"
                               "[test.cpp:28]: (warning) %Lu in format string (no. 5) requires 'unsigned long long *' but the argument type is 'short *'.\n"
                               "[test.cpp:28]: (warning) %Lu in format string (no. 6) requires 'unsigned long long *' but the argument type is 'unsigned short *'.\n"
                               "[test.cpp:28]: (warning) %Lu in format string (no. 7) requires 'unsigned long long *' but the argument type is 'int *'.\n"
                               "[test.cpp:28]: (warning) %Lu in format string (no. 8) requires 'unsigned long long *' but the argument type is 'unsigned int *'.\n"
                               "[test.cpp:28]: (warning) %Lu in format string (no. 9) requires 'unsigned long long *' but the argument type is 'long *'.\n"
                               "[test.cpp:28]: (warning) %Lu in format string (no. 10) requires 'unsigned long long *' but the argument type is 'unsigned long *'.\n"
                               "[test.cpp:28]: (warning) %Lu in format string (no. 11) requires 'unsigned long long *' but the argument type is 'long long *'.\n"
                               "[test.cpp:28]: (warning) %Lu in format string (no. 13) requires 'unsigned long long *' but the argument type is 'float *'.\n"
                               "[test.cpp:28]: (warning) %Lu in format string (no. 14) requires 'unsigned long long *' but the argument type is 'double *'.\n"
                               "[test.cpp:28]: (warning) %Lu in format string (no. 15) requires 'unsigned long long *' but the argument type is 'long double *'.\n"
                               "[test.cpp:28]: (warning) %Lu in format string (no. 16) requires 'unsigned long long *' but the argument type is 'size_t * {aka unsigned long *}'.\n"
                               "[test.cpp:28]: (warning) %Lu in format string (no. 17) requires 'unsigned long long *' but the argument type is 'ssize_t * {aka long *}'.\n"
                               "[test.cpp:28]: (warning) %Lu in format string (no. 18) requires 'unsigned long long *' but the argument type is 'ptrdiff_t * {aka long *}'.\n"
                               "[test.cpp:28]: (warning) %Lu in format string (no. 19) requires 'unsigned long long *' but the argument type is 'intmax_t * {aka long *}'.\n"
                               "[test.cpp:28]: (warning) %Lu in format string (no. 20) requires 'unsigned long long *' but the argument type is 'uintmax_t * {aka unsigned long *}'.\n"
                               "[test.cpp:28]: (warning) %Lu in format string (no. 21) requires 'unsigned long long *' but the argument type is 'void * *'.\n"
                               "[test.cpp:28]: (warning) %Lu in format string (no. 22) requires 'unsigned long long *' but the argument type is 'void *'.\n"
                               "[test.cpp:28]: (warning) %Lu in format string (no. 25) requires 'unsigned long long *' but the argument type is 'std::size_t * {aka unsigned long *}'.\n"
                               "[test.cpp:28]: (warning) %Lu in format string (no. 26) requires 'unsigned long long *' but the argument type is 'std::ssize_t * {aka long *}'.\n"
                               "[test.cpp:28]: (warning) %Lu in format string (no. 27) requires 'unsigned long long *' but the argument type is 'std::ptrdiff_t * {aka long *}'.\n"
                               "[test.cpp:28]: (warning) %Lu in format string (no. 28) requires 'unsigned long long *' but the argument type is 'std::intptr_t * {aka long *}'.\n"
                               "[test.cpp:28]: (warning) %Lu in format string (no. 29) requires 'unsigned long long *' but the argument type is 'std::uintptr_t * {aka unsigned long *}'.\n");
            std::string result_win64("[test.cpp:28]: (warning) %Lu in format string (no. 1) requires 'unsigned long long *' but the argument type is 'bool *'.\n"
                                     "[test.cpp:28]: (warning) %Lu in format string (no. 2) requires 'unsigned long long *' but the argument type is 'char *'.\n"
                                     "[test.cpp:28]: (warning) %Lu in format string (no. 3) requires 'unsigned long long *' but the argument type is 'signed char *'.\n"
                                     "[test.cpp:28]: (warning) %Lu in format string (no. 4) requires 'unsigned long long *' but the argument type is 'unsigned char *'.\n"
                                     "[test.cpp:28]: (warning) %Lu in format string (no. 5) requires 'unsigned long long *' but the argument type is 'short *'.\n"
                                     "[test.cpp:28]: (warning) %Lu in format string (no. 6) requires 'unsigned long long *' but the argument type is 'unsigned short *'.\n"
                                     "[test.cpp:28]: (warning) %Lu in format string (no. 7) requires 'unsigned long long *' but the argument type is 'int *'.\n"
                                     "[test.cpp:28]: (warning) %Lu in format string (no. 8) requires 'unsigned long long *' but the argument type is 'unsigned int *'.\n"
                                     "[test.cpp:28]: (warning) %Lu in format string (no. 9) requires 'unsigned long long *' but the argument type is 'long *'.\n"
                                     "[test.cpp:28]: (warning) %Lu in format string (no. 10) requires 'unsigned long long *' but the argument type is 'unsigned long *'.\n"
                                     "[test.cpp:28]: (warning) %Lu in format string (no. 11) requires 'unsigned long long *' but the argument type is 'long long *'.\n"
                                     "[test.cpp:28]: (warning) %Lu in format string (no. 13) requires 'unsigned long long *' but the argument type is 'float *'.\n"
                                     "[test.cpp:28]: (warning) %Lu in format string (no. 14) requires 'unsigned long long *' but the argument type is 'double *'.\n"
                                     "[test.cpp:28]: (warning) %Lu in format string (no. 15) requires 'unsigned long long *' but the argument type is 'long double *'.\n"
                                     "[test.cpp:28]: (warning) %Lu in format string (no. 16) requires 'unsigned long long *' but the argument type is 'size_t * {aka unsigned long long *}'.\n"
                                     "[test.cpp:28]: (warning) %Lu in format string (no. 17) requires 'unsigned long long *' but the argument type is 'ssize_t * {aka long long *}'.\n"
                                     "[test.cpp:28]: (warning) %Lu in format string (no. 18) requires 'unsigned long long *' but the argument type is 'ptrdiff_t * {aka long long *}'.\n"
                                     "[test.cpp:28]: (warning) %Lu in format string (no. 19) requires 'unsigned long long *' but the argument type is 'intmax_t * {aka long long *}'.\n"
                                     "[test.cpp:28]: (warning) %Lu in format string (no. 20) requires 'unsigned long long *' but the argument type is 'uintmax_t * {aka unsigned long long *}'.\n"
                                     "[test.cpp:28]: (warning) %Lu in format string (no. 21) requires 'unsigned long long *' but the argument type is 'void * *'.\n"
                                     "[test.cpp:28]: (warning) %Lu in format string (no. 22) requires 'unsigned long long *' but the argument type is 'void *'.\n"
                                     "[test.cpp:28]: (warning) %Lu in format string (no. 25) requires 'unsigned long long *' but the argument type is 'std::size_t * {aka unsigned long long *}'.\n"
                                     "[test.cpp:28]: (warning) %Lu in format string (no. 26) requires 'unsigned long long *' but the argument type is 'std::ssize_t * {aka long long *}'.\n"
                                     "[test.cpp:28]: (warning) %Lu in format string (no. 27) requires 'unsigned long long *' but the argument type is 'std::ptrdiff_t * {aka long long *}'.\n"
                                     "[test.cpp:28]: (warning) %Lu in format string (no. 28) requires 'unsigned long long *' but the argument type is 'std::intptr_t * {aka long long *}'.\n");

            check(code, true, false, Settings::Unix32);
            ASSERT_EQUALS(result, errout.str());
            check(code, true, false, Settings::Unix64);
            ASSERT_EQUALS(result, errout.str());
            check(code, true, false, Settings::Win32A);
            ASSERT_EQUALS(result, errout.str());
            check(code, true, false, Settings::Win64);
            ASSERT_EQUALS(result_win64, errout.str());
        }

        {
            const char * code = "void foo() {\n"
                                "    bool b;\n"
                                "    char c;\n"
                                "    signed char sc;\n"
                                "    unsigned char uc;\n"
                                "    short s;\n"
                                "    unsigned short us;\n"
                                "    int i;\n"
                                "    unsigned int ui;\n"
                                "    long l;\n"
                                "    unsigned long ul;\n"
                                "    long long ll;\n"
                                "    unsigned long long ull;\n"
                                "    float f;\n"
                                "    double d;\n"
                                "    long double ld;\n"
                                "    size_t st;\n"
                                "    ssize_t sst;\n"
                                "    ptrdiff_t pt;\n"
                                "    intmax_t it;\n"
                                "    uintmax_t ut;\n"
                                "    void * vp;\n"
                                "    std::size_t stdst;\n"
                                "    std::ssize_t stdsst;\n"
                                "    std::ptrdiff_t stdpt;\n"
                                "    std::intptr_t stdipt;\n"
                                "    std::uintptr_t stduipt;\n"
                                "    scanf(\"%ju %ju %ju %ju %ju %ju %ju %ju %ju %ju %ju %ju %ju %ju %ju %ju %ju %ju %ju %ju %ju %ju %ju %ju %ju %ju %ju %ju %ju\",\n"
                                "          &b, &c, &sc, &uc, &s, &us, &i, &ui, &l, &ul, &ll, &ull, &f, &d, &ld, &st, &sst, &pt, &it, &ut, &vp, vp,\n"
                                "          &unknown, unknown, &stdst, &stdsst, &stdpt, &stdipt, &stduipt);\n"
                                "}\n";

            std::string result("[test.cpp:28]: (warning) %ju in format string (no. 1) requires 'uintmax_t *' but the argument type is 'bool *'.\n"
                               "[test.cpp:28]: (warning) %ju in format string (no. 2) requires 'uintmax_t *' but the argument type is 'char *'.\n"
                               "[test.cpp:28]: (warning) %ju in format string (no. 3) requires 'uintmax_t *' but the argument type is 'signed char *'.\n"
                               "[test.cpp:28]: (warning) %ju in format string (no. 4) requires 'uintmax_t *' but the argument type is 'unsigned char *'.\n"
                               "[test.cpp:28]: (warning) %ju in format string (no. 5) requires 'uintmax_t *' but the argument type is 'short *'.\n"
                               "[test.cpp:28]: (warning) %ju in format string (no. 6) requires 'uintmax_t *' but the argument type is 'unsigned short *'.\n"
                               "[test.cpp:28]: (warning) %ju in format string (no. 7) requires 'uintmax_t *' but the argument type is 'int *'.\n"
                               "[test.cpp:28]: (warning) %ju in format string (no. 8) requires 'uintmax_t *' but the argument type is 'unsigned int *'.\n"
                               "[test.cpp:28]: (warning) %ju in format string (no. 9) requires 'uintmax_t *' but the argument type is 'long *'.\n"
                               "[test.cpp:28]: (warning) %ju in format string (no. 10) requires 'uintmax_t *' but the argument type is 'unsigned long *'.\n"
                               "[test.cpp:28]: (warning) %ju in format string (no. 11) requires 'uintmax_t *' but the argument type is 'long long *'.\n"
                               "[test.cpp:28]: (warning) %ju in format string (no. 12) requires 'uintmax_t *' but the argument type is 'unsigned long long *'.\n"
                               "[test.cpp:28]: (warning) %ju in format string (no. 13) requires 'uintmax_t *' but the argument type is 'float *'.\n"
                               "[test.cpp:28]: (warning) %ju in format string (no. 14) requires 'uintmax_t *' but the argument type is 'double *'.\n"
                               "[test.cpp:28]: (warning) %ju in format string (no. 15) requires 'uintmax_t *' but the argument type is 'long double *'.\n"
                               "[test.cpp:28]: (warning) %ju in format string (no. 16) requires 'uintmax_t *' but the argument type is 'size_t * {aka unsigned long *}'.\n"
                               "[test.cpp:28]: (warning) %ju in format string (no. 17) requires 'uintmax_t *' but the argument type is 'ssize_t * {aka long *}'.\n"
                               "[test.cpp:28]: (warning) %ju in format string (no. 18) requires 'uintmax_t *' but the argument type is 'ptrdiff_t * {aka long *}'.\n"
                               "[test.cpp:28]: (warning) %ju in format string (no. 19) requires 'uintmax_t *' but the argument type is 'intmax_t * {aka long *}'.\n"
                               "[test.cpp:28]: (warning) %ju in format string (no. 21) requires 'uintmax_t *' but the argument type is 'void * *'.\n"
                               "[test.cpp:28]: (warning) %ju in format string (no. 22) requires 'uintmax_t *' but the argument type is 'void *'.\n"
                               "[test.cpp:28]: (warning) %ju in format string (no. 25) requires 'uintmax_t *' but the argument type is 'std::size_t * {aka unsigned long *}'.\n"
                               "[test.cpp:28]: (warning) %ju in format string (no. 26) requires 'uintmax_t *' but the argument type is 'std::ssize_t * {aka long *}'.\n"
                               "[test.cpp:28]: (warning) %ju in format string (no. 27) requires 'uintmax_t *' but the argument type is 'std::ptrdiff_t * {aka long *}'.\n"
                               "[test.cpp:28]: (warning) %ju in format string (no. 28) requires 'uintmax_t *' but the argument type is 'std::intptr_t * {aka long *}'.\n"
                               "[test.cpp:28]: (warning) %ju in format string (no. 29) requires 'uintmax_t *' but the argument type is 'std::uintptr_t * {aka unsigned long *}'.\n");
            std::string result_win64("[test.cpp:28]: (warning) %ju in format string (no. 1) requires 'uintmax_t *' but the argument type is 'bool *'.\n"
                                     "[test.cpp:28]: (warning) %ju in format string (no. 2) requires 'uintmax_t *' but the argument type is 'char *'.\n"
                                     "[test.cpp:28]: (warning) %ju in format string (no. 3) requires 'uintmax_t *' but the argument type is 'signed char *'.\n"
                                     "[test.cpp:28]: (warning) %ju in format string (no. 4) requires 'uintmax_t *' but the argument type is 'unsigned char *'.\n"
                                     "[test.cpp:28]: (warning) %ju in format string (no. 5) requires 'uintmax_t *' but the argument type is 'short *'.\n"
                                     "[test.cpp:28]: (warning) %ju in format string (no. 6) requires 'uintmax_t *' but the argument type is 'unsigned short *'.\n"
                                     "[test.cpp:28]: (warning) %ju in format string (no. 7) requires 'uintmax_t *' but the argument type is 'int *'.\n"
                                     "[test.cpp:28]: (warning) %ju in format string (no. 8) requires 'uintmax_t *' but the argument type is 'unsigned int *'.\n"
                                     "[test.cpp:28]: (warning) %ju in format string (no. 9) requires 'uintmax_t *' but the argument type is 'long *'.\n"
                                     "[test.cpp:28]: (warning) %ju in format string (no. 10) requires 'uintmax_t *' but the argument type is 'unsigned long *'.\n"
                                     "[test.cpp:28]: (warning) %ju in format string (no. 11) requires 'uintmax_t *' but the argument type is 'long long *'.\n"
                                     "[test.cpp:28]: (warning) %ju in format string (no. 12) requires 'uintmax_t *' but the argument type is 'unsigned long long *'.\n"
                                     "[test.cpp:28]: (warning) %ju in format string (no. 13) requires 'uintmax_t *' but the argument type is 'float *'.\n"
                                     "[test.cpp:28]: (warning) %ju in format string (no. 14) requires 'uintmax_t *' but the argument type is 'double *'.\n"
                                     "[test.cpp:28]: (warning) %ju in format string (no. 15) requires 'uintmax_t *' but the argument type is 'long double *'.\n"
                                     "[test.cpp:28]: (warning) %ju in format string (no. 16) requires 'uintmax_t *' but the argument type is 'size_t * {aka unsigned long long *}'.\n"
                                     "[test.cpp:28]: (warning) %ju in format string (no. 17) requires 'uintmax_t *' but the argument type is 'ssize_t * {aka long long *}'.\n"
                                     "[test.cpp:28]: (warning) %ju in format string (no. 18) requires 'uintmax_t *' but the argument type is 'ptrdiff_t * {aka long long *}'.\n"
                                     "[test.cpp:28]: (warning) %ju in format string (no. 19) requires 'uintmax_t *' but the argument type is 'intmax_t * {aka long long *}'.\n"
                                     "[test.cpp:28]: (warning) %ju in format string (no. 21) requires 'uintmax_t *' but the argument type is 'void * *'.\n"
                                     "[test.cpp:28]: (warning) %ju in format string (no. 22) requires 'uintmax_t *' but the argument type is 'void *'.\n"
                                     "[test.cpp:28]: (warning) %ju in format string (no. 25) requires 'uintmax_t *' but the argument type is 'std::size_t * {aka unsigned long long *}'.\n"
                                     "[test.cpp:28]: (warning) %ju in format string (no. 26) requires 'uintmax_t *' but the argument type is 'std::ssize_t * {aka long long *}'.\n"
                                     "[test.cpp:28]: (warning) %ju in format string (no. 27) requires 'uintmax_t *' but the argument type is 'std::ptrdiff_t * {aka long long *}'.\n"
                                     "[test.cpp:28]: (warning) %ju in format string (no. 28) requires 'uintmax_t *' but the argument type is 'std::intptr_t * {aka long long *}'.\n"
                                     "[test.cpp:28]: (warning) %ju in format string (no. 29) requires 'uintmax_t *' but the argument type is 'std::uintptr_t * {aka unsigned long long *}'.\n");

            check(code, true, false, Settings::Unix32);
            ASSERT_EQUALS(result, errout.str());
            check(code, true, false, Settings::Unix64);
            ASSERT_EQUALS(result, errout.str());
            check(code, true, false, Settings::Win32A);
            ASSERT_EQUALS(result, errout.str());
            check(code, true, false, Settings::Win64);
            ASSERT_EQUALS(result_win64, errout.str());
        }

        {
            const char * code = "void foo() {\n"
                                "    bool b;\n"
                                "    char c;\n"
                                "    signed char sc;\n"
                                "    unsigned char uc;\n"
                                "    short s;\n"
                                "    unsigned short us;\n"
                                "    int i;\n"
                                "    unsigned int ui;\n"
                                "    long l;\n"
                                "    unsigned long ul;\n"
                                "    long long ll;\n"
                                "    unsigned long long ull;\n"
                                "    float f;\n"
                                "    double d;\n"
                                "    long double ld;\n"
                                "    size_t st;\n"
                                "    ssize_t sst;\n"
                                "    ptrdiff_t pt;\n"
                                "    intmax_t it;\n"
                                "    uintmax_t ut;\n"
                                "    void * vp;\n"
                                "    std::size_t stdst;\n"
                                "    std::ssize_t stdsst;\n"
                                "    std::ptrdiff_t stdpt;\n"
                                "    std::intptr_t stdipt;\n"
                                "    std::uintptr_t stduipt;\n"
                                "    scanf(\"%zu %zu %zu %zu %zu %zu %zu %zu %zu %zu %zu %zu %zu %zu %zu %zu %zu %zu %zu %zu %zu %zu %zu %zu %zu %zu %zu %zu %zu\",\n"
                                "          &b, &c, &sc, &uc, &s, &us, &i, &ui, &l, &ul, &ll, &ull, &f, &d, &ld, &st, &sst, &pt, &it, &ut, &vp, vp,\n"
                                "          &unknown, unknown, &stdst, &stdsst, &stdpt, &stdipt, &stduipt);\n"
                                "}\n";
            std::string result("[test.cpp:28]: (warning) %zu in format string (no. 1) requires 'size_t *' but the argument type is 'bool *'.\n"
                               "[test.cpp:28]: (warning) %zu in format string (no. 2) requires 'size_t *' but the argument type is 'char *'.\n"
                               "[test.cpp:28]: (warning) %zu in format string (no. 3) requires 'size_t *' but the argument type is 'signed char *'.\n"
                               "[test.cpp:28]: (warning) %zu in format string (no. 4) requires 'size_t *' but the argument type is 'unsigned char *'.\n"
                               "[test.cpp:28]: (warning) %zu in format string (no. 5) requires 'size_t *' but the argument type is 'short *'.\n"
                               "[test.cpp:28]: (warning) %zu in format string (no. 6) requires 'size_t *' but the argument type is 'unsigned short *'.\n"
                               "[test.cpp:28]: (warning) %zu in format string (no. 7) requires 'size_t *' but the argument type is 'int *'.\n"
                               "[test.cpp:28]: (warning) %zu in format string (no. 8) requires 'size_t *' but the argument type is 'unsigned int *'.\n"
                               "[test.cpp:28]: (warning) %zu in format string (no. 9) requires 'size_t *' but the argument type is 'long *'.\n"
                               "[test.cpp:28]: (warning) %zu in format string (no. 10) requires 'size_t *' but the argument type is 'unsigned long *'.\n"
                               "[test.cpp:28]: (warning) %zu in format string (no. 11) requires 'size_t *' but the argument type is 'long long *'.\n"
                               "[test.cpp:28]: (warning) %zu in format string (no. 12) requires 'size_t *' but the argument type is 'unsigned long long *'.\n"
                               "[test.cpp:28]: (warning) %zu in format string (no. 13) requires 'size_t *' but the argument type is 'float *'.\n"
                               "[test.cpp:28]: (warning) %zu in format string (no. 14) requires 'size_t *' but the argument type is 'double *'.\n"
                               "[test.cpp:28]: (warning) %zu in format string (no. 15) requires 'size_t *' but the argument type is 'long double *'.\n"
                               "[test.cpp:28]: (warning) %zu in format string (no. 17) requires 'size_t *' but the argument type is 'ssize_t * {aka long *}'.\n"
                               "[test.cpp:28]: (warning) %zu in format string (no. 18) requires 'size_t *' but the argument type is 'ptrdiff_t * {aka long *}'.\n"
                               "[test.cpp:28]: (warning) %zu in format string (no. 19) requires 'size_t *' but the argument type is 'intmax_t * {aka long *}'.\n"
                               "[test.cpp:28]: (warning) %zu in format string (no. 20) requires 'size_t *' but the argument type is 'uintmax_t * {aka unsigned long *}'.\n"
                               "[test.cpp:28]: (warning) %zu in format string (no. 21) requires 'size_t *' but the argument type is 'void * *'.\n"
                               "[test.cpp:28]: (warning) %zu in format string (no. 22) requires 'size_t *' but the argument type is 'void *'.\n"
                               "[test.cpp:28]: (warning) %zu in format string (no. 26) requires 'size_t *' but the argument type is 'std::ssize_t * {aka long *}'.\n"
                               "[test.cpp:28]: (warning) %zu in format string (no. 27) requires 'size_t *' but the argument type is 'std::ptrdiff_t * {aka long *}'.\n"
                               "[test.cpp:28]: (warning) %zu in format string (no. 28) requires 'size_t *' but the argument type is 'std::intptr_t * {aka long *}'.\n"
                               "[test.cpp:28]: (warning) %zu in format string (no. 29) requires 'size_t *' but the argument type is 'std::uintptr_t * {aka unsigned long *}'.\n");
            std::string result_win64("[test.cpp:28]: (warning) %zu in format string (no. 1) requires 'size_t *' but the argument type is 'bool *'.\n"
                                     "[test.cpp:28]: (warning) %zu in format string (no. 2) requires 'size_t *' but the argument type is 'char *'.\n"
                                     "[test.cpp:28]: (warning) %zu in format string (no. 3) requires 'size_t *' but the argument type is 'signed char *'.\n"
                                     "[test.cpp:28]: (warning) %zu in format string (no. 4) requires 'size_t *' but the argument type is 'unsigned char *'.\n"
                                     "[test.cpp:28]: (warning) %zu in format string (no. 5) requires 'size_t *' but the argument type is 'short *'.\n"
                                     "[test.cpp:28]: (warning) %zu in format string (no. 6) requires 'size_t *' but the argument type is 'unsigned short *'.\n"
                                     "[test.cpp:28]: (warning) %zu in format string (no. 7) requires 'size_t *' but the argument type is 'int *'.\n"
                                     "[test.cpp:28]: (warning) %zu in format string (no. 8) requires 'size_t *' but the argument type is 'unsigned int *'.\n"
                                     "[test.cpp:28]: (warning) %zu in format string (no. 9) requires 'size_t *' but the argument type is 'long *'.\n"
                                     "[test.cpp:28]: (warning) %zu in format string (no. 10) requires 'size_t *' but the argument type is 'unsigned long *'.\n"
                                     "[test.cpp:28]: (warning) %zu in format string (no. 11) requires 'size_t *' but the argument type is 'long long *'.\n"
                                     "[test.cpp:28]: (warning) %zu in format string (no. 12) requires 'size_t *' but the argument type is 'unsigned long long *'.\n"
                                     "[test.cpp:28]: (warning) %zu in format string (no. 13) requires 'size_t *' but the argument type is 'float *'.\n"
                                     "[test.cpp:28]: (warning) %zu in format string (no. 14) requires 'size_t *' but the argument type is 'double *'.\n"
                                     "[test.cpp:28]: (warning) %zu in format string (no. 15) requires 'size_t *' but the argument type is 'long double *'.\n"
                                     "[test.cpp:28]: (warning) %zu in format string (no. 17) requires 'size_t *' but the argument type is 'ssize_t * {aka long long *}'.\n"
                                     "[test.cpp:28]: (warning) %zu in format string (no. 18) requires 'size_t *' but the argument type is 'ptrdiff_t * {aka long long *}'.\n"
                                     "[test.cpp:28]: (warning) %zu in format string (no. 19) requires 'size_t *' but the argument type is 'intmax_t * {aka long long *}'.\n"
                                     "[test.cpp:28]: (warning) %zu in format string (no. 20) requires 'size_t *' but the argument type is 'uintmax_t * {aka unsigned long long *}'.\n"
                                     "[test.cpp:28]: (warning) %zu in format string (no. 21) requires 'size_t *' but the argument type is 'void * *'.\n"
                                     "[test.cpp:28]: (warning) %zu in format string (no. 22) requires 'size_t *' but the argument type is 'void *'.\n"
                                     "[test.cpp:28]: (warning) %zu in format string (no. 26) requires 'size_t *' but the argument type is 'std::ssize_t * {aka long long *}'.\n"
                                     "[test.cpp:28]: (warning) %zu in format string (no. 27) requires 'size_t *' but the argument type is 'std::ptrdiff_t * {aka long long *}'.\n"
                                     "[test.cpp:28]: (warning) %zu in format string (no. 28) requires 'size_t *' but the argument type is 'std::intptr_t * {aka long long *}'.\n"
                                     "[test.cpp:28]: (warning) %zu in format string (no. 29) requires 'size_t *' but the argument type is 'std::uintptr_t * {aka unsigned long long *}'.\n");

            check(code, true, false, Settings::Unix32);
            ASSERT_EQUALS(result, errout.str());
            check(code, true, false, Settings::Unix64);
            ASSERT_EQUALS(result, errout.str());
            check(code, true, false, Settings::Win32A);
            ASSERT_EQUALS(result, errout.str());
            check(code, true, false, Settings::Win64);
            ASSERT_EQUALS(result_win64, errout.str());
        }

        check("void foo() {\n"
              "    bool b;\n"
              "    char c;\n"
              "    signed char sc;\n"
              "    unsigned char uc;\n"
              "    short s;\n"
              "    unsigned short us;\n"
              "    int i;\n"
              "    unsigned int ui;\n"
              "    long l;\n"
              "    unsigned long ul;\n"
              "    long long ll;\n"
              "    unsigned long long ull;\n"
              "    float f;\n"
              "    double d;\n"
              "    long double ld;\n"
              "    size_t st;\n"
              "    ssize_t sst;\n"
              "    ptrdiff_t pt;\n"
              "    intmax_t it;\n"
              "    uintmax_t ut;\n"
              "    void * vp;\n"
              "    std::size_t stdst;\n"
              "    std::ssize_t stdsst;\n"
              "    std::ptrdiff_t stdpt;\n"
              "    std::intptr_t stdipt;\n"
              "    std::uintptr_t stduipt;\n"
              "    scanf(\"%tu %tu %tu %tu %tu %tu %tu %tu %tu %tu %tu %tu %tu %tu %tu %tu %tu %tu %tu %tu %tu %tu %tu %tu %tu %tu %tu %tu %tu\",\n"
              "          &b, &c, &sc, &uc, &s, &us, &i, &ui, &l, &ul, &ll, &ull, &f, &d, &ld, &st, &sst, &pt, &it, &ut, &vp, vp,\n"
              "          &unknown, unknown, &stdst, &stdsst, &stdpt, &stdipt, &stduipt);\n"
              "}\n", false, false, Settings::Unix64);
        ASSERT_EQUALS("[test.cpp:28]: (warning) %tu in format string (no. 1) requires 'unsigned ptrdiff_t *' but the argument type is 'bool *'.\n"
                      "[test.cpp:28]: (warning) %tu in format string (no. 2) requires 'unsigned ptrdiff_t *' but the argument type is 'char *'.\n"
                      "[test.cpp:28]: (warning) %tu in format string (no. 3) requires 'unsigned ptrdiff_t *' but the argument type is 'signed char *'.\n"
                      "[test.cpp:28]: (warning) %tu in format string (no. 4) requires 'unsigned ptrdiff_t *' but the argument type is 'unsigned char *'.\n"
                      "[test.cpp:28]: (warning) %tu in format string (no. 5) requires 'unsigned ptrdiff_t *' but the argument type is 'short *'.\n"
                      "[test.cpp:28]: (warning) %tu in format string (no. 6) requires 'unsigned ptrdiff_t *' but the argument type is 'unsigned short *'.\n"
                      "[test.cpp:28]: (warning) %tu in format string (no. 7) requires 'unsigned ptrdiff_t *' but the argument type is 'int *'.\n"
                      "[test.cpp:28]: (warning) %tu in format string (no. 8) requires 'unsigned ptrdiff_t *' but the argument type is 'unsigned int *'.\n"
                      "[test.cpp:28]: (warning) %tu in format string (no. 9) requires 'unsigned ptrdiff_t *' but the argument type is 'long *'.\n"
                      "[test.cpp:28]: (warning) %tu in format string (no. 10) requires 'unsigned ptrdiff_t *' but the argument type is 'unsigned long *'.\n"
                      "[test.cpp:28]: (warning) %tu in format string (no. 11) requires 'unsigned ptrdiff_t *' but the argument type is 'long long *'.\n"
                      "[test.cpp:28]: (warning) %tu in format string (no. 12) requires 'unsigned ptrdiff_t *' but the argument type is 'unsigned long long *'.\n"
                      "[test.cpp:28]: (warning) %tu in format string (no. 13) requires 'unsigned ptrdiff_t *' but the argument type is 'float *'.\n"
                      "[test.cpp:28]: (warning) %tu in format string (no. 14) requires 'unsigned ptrdiff_t *' but the argument type is 'double *'.\n"
                      "[test.cpp:28]: (warning) %tu in format string (no. 15) requires 'unsigned ptrdiff_t *' but the argument type is 'long double *'.\n"
                      "[test.cpp:28]: (warning) %tu in format string (no. 16) requires 'unsigned ptrdiff_t *' but the argument type is 'size_t * {aka unsigned long *}'.\n"
                      "[test.cpp:28]: (warning) %tu in format string (no. 17) requires 'unsigned ptrdiff_t *' but the argument type is 'ssize_t * {aka long *}'.\n"
                      "[test.cpp:28]: (warning) %tu in format string (no. 18) requires 'unsigned ptrdiff_t *' but the argument type is 'ptrdiff_t * {aka long *}'.\n"
                      "[test.cpp:28]: (warning) %tu in format string (no. 19) requires 'unsigned ptrdiff_t *' but the argument type is 'intmax_t * {aka long *}'.\n"
                      "[test.cpp:28]: (warning) %tu in format string (no. 20) requires 'unsigned ptrdiff_t *' but the argument type is 'uintmax_t * {aka unsigned long *}'.\n"
                      "[test.cpp:28]: (warning) %tu in format string (no. 21) requires 'unsigned ptrdiff_t *' but the argument type is 'void * *'.\n"
                      "[test.cpp:28]: (warning) %tu in format string (no. 22) requires 'unsigned ptrdiff_t *' but the argument type is 'void *'.\n"
                      "[test.cpp:28]: (warning) %tu in format string (no. 25) requires 'unsigned ptrdiff_t *' but the argument type is 'std::size_t * {aka unsigned long *}'.\n"
                      "[test.cpp:28]: (warning) %tu in format string (no. 26) requires 'unsigned ptrdiff_t *' but the argument type is 'std::ssize_t * {aka long *}'.\n"
                      "[test.cpp:28]: (warning) %tu in format string (no. 27) requires 'unsigned ptrdiff_t *' but the argument type is 'std::ptrdiff_t * {aka long *}'.\n"
                      "[test.cpp:28]: (warning) %tu in format string (no. 28) requires 'unsigned ptrdiff_t *' but the argument type is 'std::intptr_t * {aka long *}'.\n"
                      "[test.cpp:28]: (warning) %tu in format string (no. 29) requires 'unsigned ptrdiff_t *' but the argument type is 'std::uintptr_t * {aka unsigned long *}'.\n", errout.str());

        check("void foo() {\n"
              "    bool b;\n"
              "    char c;\n"
              "    signed char sc;\n"
              "    unsigned char uc;\n"
              "    short s;\n"
              "    unsigned short us;\n"
              "    int i;\n"
              "    unsigned int ui;\n"
              "    long l;\n"
              "    unsigned long ul;\n"
              "    long long ll;\n"
              "    unsigned long long ull;\n"
              "    float f;\n"
              "    double d;\n"
              "    long double ld;\n"
              "    size_t st;\n"
              "    ssize_t sst;\n"
              "    ptrdiff_t pt;\n"
              "    intmax_t it;\n"
              "    uintmax_t ut;\n"
              "    void * vp;\n"
              "    std::size_t stdst;\n"
              "    std::ssize_t stdsst;\n"
              "    std::ptrdiff_t stdpt;\n"
              "    std::intptr_t stdipt;\n"
              "    std::uintptr_t stduipt;\n"
              "    scanf(\"%I64u %I64u %I64u %I64u %I64u %I64u %I64u %I64u %I64u %I64u %I64u %I64u %I64u %I64u %I64u %I64u %I64u %I64u %I64u %I64u %I64u %I64u %I64u %I64u %I64u %I64u %I64u %I64u %I64u\",\n"
              "          &b, &c, &sc, &uc, &s, &us, &i, &ui, &l, &ul, &ll, &ull, &f, &d, &ld, &st, &sst, &pt, &it, &ut, &vp, vp,\n"
              "          &unknown, unknown, &stdst, &stdsst, &stdpt, &stdipt, &stduipt);\n"
              "}\n", false, false, Settings::Unix64);
        ASSERT_EQUALS("[test.cpp:28]: (warning) %I64u in format string (no. 1) requires 'unsigned __int64 *' but the argument type is 'bool *'.\n"
                      "[test.cpp:28]: (warning) %I64u in format string (no. 2) requires 'unsigned __int64 *' but the argument type is 'char *'.\n"
                      "[test.cpp:28]: (warning) %I64u in format string (no. 3) requires 'unsigned __int64 *' but the argument type is 'signed char *'.\n"
                      "[test.cpp:28]: (warning) %I64u in format string (no. 4) requires 'unsigned __int64 *' but the argument type is 'unsigned char *'.\n"
                      "[test.cpp:28]: (warning) %I64u in format string (no. 5) requires 'unsigned __int64 *' but the argument type is 'short *'.\n"
                      "[test.cpp:28]: (warning) %I64u in format string (no. 6) requires 'unsigned __int64 *' but the argument type is 'unsigned short *'.\n"
                      "[test.cpp:28]: (warning) %I64u in format string (no. 7) requires 'unsigned __int64 *' but the argument type is 'int *'.\n"
                      "[test.cpp:28]: (warning) %I64u in format string (no. 8) requires 'unsigned __int64 *' but the argument type is 'unsigned int *'.\n"
                      "[test.cpp:28]: (warning) %I64u in format string (no. 9) requires 'unsigned __int64 *' but the argument type is 'long *'.\n"
                      "[test.cpp:28]: (warning) %I64u in format string (no. 10) requires 'unsigned __int64 *' but the argument type is 'unsigned long *'.\n"
                      "[test.cpp:28]: (warning) %I64u in format string (no. 11) requires 'unsigned __int64 *' but the argument type is 'long long *'.\n"
                      "[test.cpp:28]: (warning) %I64u in format string (no. 13) requires 'unsigned __int64 *' but the argument type is 'float *'.\n"
                      "[test.cpp:28]: (warning) %I64u in format string (no. 14) requires 'unsigned __int64 *' but the argument type is 'double *'.\n"
                      "[test.cpp:28]: (warning) %I64u in format string (no. 15) requires 'unsigned __int64 *' but the argument type is 'long double *'.\n"
                      "[test.cpp:28]: (warning) %I64u in format string (no. 16) requires 'unsigned __int64 *' but the argument type is 'size_t * {aka unsigned long *}'.\n"
                      "[test.cpp:28]: (warning) %I64u in format string (no. 17) requires 'unsigned __int64 *' but the argument type is 'ssize_t * {aka long *}'.\n"
                      "[test.cpp:28]: (warning) %I64u in format string (no. 18) requires 'unsigned __int64 *' but the argument type is 'ptrdiff_t * {aka long *}'.\n"
                      "[test.cpp:28]: (warning) %I64u in format string (no. 19) requires 'unsigned __int64 *' but the argument type is 'intmax_t * {aka long *}'.\n"
                      "[test.cpp:28]: (warning) %I64u in format string (no. 20) requires 'unsigned __int64 *' but the argument type is 'uintmax_t * {aka unsigned long *}'.\n"
                      "[test.cpp:28]: (warning) %I64u in format string (no. 21) requires 'unsigned __int64 *' but the argument type is 'void * *'.\n"
                      "[test.cpp:28]: (warning) %I64u in format string (no. 22) requires 'unsigned __int64 *' but the argument type is 'void *'.\n"
                      "[test.cpp:28]: (warning) %I64u in format string (no. 25) requires 'unsigned __int64 *' but the argument type is 'std::size_t * {aka unsigned long *}'.\n"
                      "[test.cpp:28]: (warning) %I64u in format string (no. 26) requires 'unsigned __int64 *' but the argument type is 'std::ssize_t * {aka long *}'.\n"
                      "[test.cpp:28]: (warning) %I64u in format string (no. 27) requires 'unsigned __int64 *' but the argument type is 'std::ptrdiff_t * {aka long *}'.\n"
                      "[test.cpp:28]: (warning) %I64u in format string (no. 28) requires 'unsigned __int64 *' but the argument type is 'std::intptr_t * {aka long *}'.\n"
                      "[test.cpp:28]: (warning) %I64u in format string (no. 29) requires 'unsigned __int64 *' but the argument type is 'std::uintptr_t * {aka unsigned long *}'.\n", errout.str());

        {
            const char * code = "const int * foo() { }\n"
                                "void foo() {\n"
                                "    bool b;\n"
                                "    char c;\n"
                                "    signed char sc;\n"
                                "    unsigned char uc;\n"
                                "    short s;\n"
                                "    unsigned short us;\n"
                                "    int i;\n"
                                "    unsigned int ui;\n"
                                "    long l;\n"
                                "    unsigned long ul;\n"
                                "    long long ll;\n"
                                "    unsigned long long ull;\n"
                                "    float f;\n"
                                "    double d;\n"
                                "    long double ld;\n"
                                "    size_t st;\n"
                                "    ssize_t sst;\n"
                                "    ptrdiff_t pt;\n"
                                "    intmax_t it;\n"
                                "    uintmax_t ut;\n"
                                "    void * vp;\n"
                                "    std::size_t stdst;\n"
                                "    std::ssize_t stdsst;\n"
                                "    std::ptrdiff_t stdpt;\n"
                                "    std::intptr_t stdipt;\n"
                                "    std::uintptr_t stduipt;\n"
                                "    scanf(\"%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\",\n"
                                "          &b, &c, &sc, &uc, &s, &us, &i, &ui, &l, &ul, &ll, &ull, &f, &d, &ld, &st, &sst, &pt, &it, &ut, &vp, vp,\n"
                                "          \"str\", L\"str\", foo(), &unknown, unknown, &stdst, &stdsst, &stdpt, &stdipt, &stduipt);\n"
                                "}\n";
            std::string result("[test.cpp:29]: (warning) %d in format string (no. 1) requires 'int *' but the argument type is 'bool *'.\n"
                               "[test.cpp:29]: (warning) %d in format string (no. 2) requires 'int *' but the argument type is 'char *'.\n"
                               "[test.cpp:29]: (warning) %d in format string (no. 3) requires 'int *' but the argument type is 'signed char *'.\n"
                               "[test.cpp:29]: (warning) %d in format string (no. 4) requires 'int *' but the argument type is 'unsigned char *'.\n"
                               "[test.cpp:29]: (warning) %d in format string (no. 5) requires 'int *' but the argument type is 'short *'.\n"
                               "[test.cpp:29]: (warning) %d in format string (no. 6) requires 'int *' but the argument type is 'unsigned short *'.\n"
                               "[test.cpp:29]: (warning) %d in format string (no. 8) requires 'int *' but the argument type is 'unsigned int *'.\n"
                               "[test.cpp:29]: (warning) %d in format string (no. 9) requires 'int *' but the argument type is 'long *'.\n"
                               "[test.cpp:29]: (warning) %d in format string (no. 10) requires 'int *' but the argument type is 'unsigned long *'.\n"
                               "[test.cpp:29]: (warning) %d in format string (no. 11) requires 'int *' but the argument type is 'long long *'.\n"
                               "[test.cpp:29]: (warning) %d in format string (no. 12) requires 'int *' but the argument type is 'unsigned long long *'.\n"
                               "[test.cpp:29]: (warning) %d in format string (no. 13) requires 'int *' but the argument type is 'float *'.\n"
                               "[test.cpp:29]: (warning) %d in format string (no. 14) requires 'int *' but the argument type is 'double *'.\n"
                               "[test.cpp:29]: (warning) %d in format string (no. 15) requires 'int *' but the argument type is 'long double *'.\n"
                               "[test.cpp:29]: (warning) %d in format string (no. 16) requires 'int *' but the argument type is 'size_t * {aka unsigned long *}'.\n"
                               "[test.cpp:29]: (warning) %d in format string (no. 17) requires 'int *' but the argument type is 'ssize_t * {aka long *}'.\n"
                               "[test.cpp:29]: (warning) %d in format string (no. 18) requires 'int *' but the argument type is 'ptrdiff_t * {aka long *}'.\n"
                               "[test.cpp:29]: (warning) %d in format string (no. 19) requires 'int *' but the argument type is 'intmax_t * {aka long *}'.\n"
                               "[test.cpp:29]: (warning) %d in format string (no. 20) requires 'int *' but the argument type is 'uintmax_t * {aka unsigned long *}'.\n"
                               "[test.cpp:29]: (warning) %d in format string (no. 21) requires 'int *' but the argument type is 'void * *'.\n"
                               "[test.cpp:29]: (warning) %d in format string (no. 22) requires 'int *' but the argument type is 'void *'.\n"
                               "[test.cpp:29]: (warning) %d in format string (no. 23) requires 'int *' but the argument type is 'const char *'.\n"
                               "[test.cpp:29]: (warning) %d in format string (no. 24) requires 'int *' but the argument type is 'const wchar_t *'.\n"
                               "[test.cpp:29]: (warning) %d in format string (no. 25) requires 'int *' but the argument type is 'const int *'.\n"
                               "[test.cpp:29]: (warning) %d in format string (no. 28) requires 'int *' but the argument type is 'std::size_t * {aka unsigned long *}'.\n"
                               "[test.cpp:29]: (warning) %d in format string (no. 29) requires 'int *' but the argument type is 'std::ssize_t * {aka long *}'.\n"
                               "[test.cpp:29]: (warning) %d in format string (no. 30) requires 'int *' but the argument type is 'std::ptrdiff_t * {aka long *}'.\n"
                               "[test.cpp:29]: (warning) %d in format string (no. 31) requires 'int *' but the argument type is 'std::intptr_t * {aka long *}'.\n"
                               "[test.cpp:29]: (warning) %d in format string (no. 32) requires 'int *' but the argument type is 'std::uintptr_t * {aka unsigned long *}'.\n");
            std::string result_win64("[test.cpp:29]: (warning) %d in format string (no. 1) requires 'int *' but the argument type is 'bool *'.\n"
                                     "[test.cpp:29]: (warning) %d in format string (no. 2) requires 'int *' but the argument type is 'char *'.\n"
                                     "[test.cpp:29]: (warning) %d in format string (no. 3) requires 'int *' but the argument type is 'signed char *'.\n"
                                     "[test.cpp:29]: (warning) %d in format string (no. 4) requires 'int *' but the argument type is 'unsigned char *'.\n"
                                     "[test.cpp:29]: (warning) %d in format string (no. 5) requires 'int *' but the argument type is 'short *'.\n"
                                     "[test.cpp:29]: (warning) %d in format string (no. 6) requires 'int *' but the argument type is 'unsigned short *'.\n"
                                     "[test.cpp:29]: (warning) %d in format string (no. 8) requires 'int *' but the argument type is 'unsigned int *'.\n"
                                     "[test.cpp:29]: (warning) %d in format string (no. 9) requires 'int *' but the argument type is 'long *'.\n"
                                     "[test.cpp:29]: (warning) %d in format string (no. 10) requires 'int *' but the argument type is 'unsigned long *'.\n"
                                     "[test.cpp:29]: (warning) %d in format string (no. 11) requires 'int *' but the argument type is 'long long *'.\n"
                                     "[test.cpp:29]: (warning) %d in format string (no. 12) requires 'int *' but the argument type is 'unsigned long long *'.\n"
                                     "[test.cpp:29]: (warning) %d in format string (no. 13) requires 'int *' but the argument type is 'float *'.\n"
                                     "[test.cpp:29]: (warning) %d in format string (no. 14) requires 'int *' but the argument type is 'double *'.\n"
                                     "[test.cpp:29]: (warning) %d in format string (no. 15) requires 'int *' but the argument type is 'long double *'.\n"
                                     "[test.cpp:29]: (warning) %d in format string (no. 16) requires 'int *' but the argument type is 'size_t * {aka unsigned long long *}'.\n"
                                     "[test.cpp:29]: (warning) %d in format string (no. 17) requires 'int *' but the argument type is 'ssize_t * {aka long long *}'.\n"
                                     "[test.cpp:29]: (warning) %d in format string (no. 18) requires 'int *' but the argument type is 'ptrdiff_t * {aka long long *}'.\n"
                                     "[test.cpp:29]: (warning) %d in format string (no. 19) requires 'int *' but the argument type is 'intmax_t * {aka long long *}'.\n"
                                     "[test.cpp:29]: (warning) %d in format string (no. 20) requires 'int *' but the argument type is 'uintmax_t * {aka unsigned long long *}'.\n"
                                     "[test.cpp:29]: (warning) %d in format string (no. 21) requires 'int *' but the argument type is 'void * *'.\n"
                                     "[test.cpp:29]: (warning) %d in format string (no. 22) requires 'int *' but the argument type is 'void *'.\n"
                                     "[test.cpp:29]: (warning) %d in format string (no. 23) requires 'int *' but the argument type is 'const char *'.\n"
                                     "[test.cpp:29]: (warning) %d in format string (no. 24) requires 'int *' but the argument type is 'const wchar_t *'.\n"
                                     "[test.cpp:29]: (warning) %d in format string (no. 25) requires 'int *' but the argument type is 'const int *'.\n"
                                     "[test.cpp:29]: (warning) %d in format string (no. 28) requires 'int *' but the argument type is 'std::size_t * {aka unsigned long long *}'.\n"
                                     "[test.cpp:29]: (warning) %d in format string (no. 29) requires 'int *' but the argument type is 'std::ssize_t * {aka long long *}'.\n"
                                     "[test.cpp:29]: (warning) %d in format string (no. 30) requires 'int *' but the argument type is 'std::ptrdiff_t * {aka long long *}'.\n"
                                     "[test.cpp:29]: (warning) %d in format string (no. 31) requires 'int *' but the argument type is 'std::intptr_t * {aka long long *}'.\n"
                                     "[test.cpp:29]: (warning) %d in format string (no. 32) requires 'int *' but the argument type is 'std::uintptr_t * {aka unsigned long long *}'.\n");

            check(code, true, false, Settings::Unix32);
            ASSERT_EQUALS(result, errout.str());
            check(code, true, false, Settings::Unix64);
            ASSERT_EQUALS(result, errout.str());
            check(code, true, false, Settings::Win32A);
            ASSERT_EQUALS(result, errout.str());
            check(code, true, false, Settings::Win64);
            ASSERT_EQUALS(result_win64, errout.str());
        }

        {
            const char * code = "const unsigned int * foo() { }\n"
                                "void foo() {\n"
                                "    bool b;\n"
                                "    char c;\n"
                                "    signed char sc;\n"
                                "    unsigned char uc;\n"
                                "    short s;\n"
                                "    unsigned short us;\n"
                                "    int i;\n"
                                "    unsigned int ui;\n"
                                "    long l;\n"
                                "    unsigned long ul;\n"
                                "    long long ll;\n"
                                "    unsigned long long ull;\n"
                                "    float f;\n"
                                "    double d;\n"
                                "    long double ld;\n"
                                "    size_t st;\n"
                                "    ssize_t sst;\n"
                                "    ptrdiff_t pt;\n"
                                "    intmax_t it;\n"
                                "    uintmax_t ut;\n"
                                "    void * vp;\n"
                                "    std::size_t stdst;\n"
                                "    std::ssize_t stdsst;\n"
                                "    std::ptrdiff_t stdpt;\n"
                                "    std::intptr_t stdipt;\n"
                                "    std::uintptr_t stduipt;\n"
                                "    scanf(\"%x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x\",\n"
                                "          &b, &c, &sc, &uc, &s, &us, &i, &ui, &l, &ul, &ll, &ull, &f, &d, &ld, &st, &sst, &pt, &it, &ut, &vp, vp,\n"
                                "          \"str\", L\"str\", foo(), &unknown, unknown, &stdst, &stdsst, &stdpt, &stdipt, &stduipt);\n"
                                "}\n";
            std::string result("[test.cpp:29]: (warning) %x in format string (no. 1) requires 'unsigned int *' but the argument type is 'bool *'.\n"
                               "[test.cpp:29]: (warning) %x in format string (no. 2) requires 'unsigned int *' but the argument type is 'char *'.\n"
                               "[test.cpp:29]: (warning) %x in format string (no. 3) requires 'unsigned int *' but the argument type is 'signed char *'.\n"
                               "[test.cpp:29]: (warning) %x in format string (no. 4) requires 'unsigned int *' but the argument type is 'unsigned char *'.\n"
                               "[test.cpp:29]: (warning) %x in format string (no. 5) requires 'unsigned int *' but the argument type is 'short *'.\n"
                               "[test.cpp:29]: (warning) %x in format string (no. 6) requires 'unsigned int *' but the argument type is 'unsigned short *'.\n"
                               "[test.cpp:29]: (warning) %x in format string (no. 9) requires 'unsigned int *' but the argument type is 'long *'.\n"
                               "[test.cpp:29]: (warning) %x in format string (no. 10) requires 'unsigned int *' but the argument type is 'unsigned long *'.\n"
                               "[test.cpp:29]: (warning) %x in format string (no. 11) requires 'unsigned int *' but the argument type is 'long long *'.\n"
                               "[test.cpp:29]: (warning) %x in format string (no. 12) requires 'unsigned int *' but the argument type is 'unsigned long long *'.\n"
                               "[test.cpp:29]: (warning) %x in format string (no. 13) requires 'unsigned int *' but the argument type is 'float *'.\n"
                               "[test.cpp:29]: (warning) %x in format string (no. 14) requires 'unsigned int *' but the argument type is 'double *'.\n"
                               "[test.cpp:29]: (warning) %x in format string (no. 15) requires 'unsigned int *' but the argument type is 'long double *'.\n"
                               "[test.cpp:29]: (warning) %x in format string (no. 16) requires 'unsigned int *' but the argument type is 'size_t * {aka unsigned long *}'.\n"
                               "[test.cpp:29]: (warning) %x in format string (no. 17) requires 'unsigned int *' but the argument type is 'ssize_t * {aka long *}'.\n"
                               "[test.cpp:29]: (warning) %x in format string (no. 18) requires 'unsigned int *' but the argument type is 'ptrdiff_t * {aka long *}'.\n"
                               "[test.cpp:29]: (warning) %x in format string (no. 19) requires 'unsigned int *' but the argument type is 'intmax_t * {aka long *}'.\n"
                               "[test.cpp:29]: (warning) %x in format string (no. 20) requires 'unsigned int *' but the argument type is 'uintmax_t * {aka unsigned long *}'.\n"
                               "[test.cpp:29]: (warning) %x in format string (no. 21) requires 'unsigned int *' but the argument type is 'void * *'.\n"
                               "[test.cpp:29]: (warning) %x in format string (no. 22) requires 'unsigned int *' but the argument type is 'void *'.\n"
                               "[test.cpp:29]: (warning) %x in format string (no. 23) requires 'unsigned int *' but the argument type is 'const char *'.\n"
                               "[test.cpp:29]: (warning) %x in format string (no. 24) requires 'unsigned int *' but the argument type is 'const wchar_t *'.\n"
                               "[test.cpp:29]: (warning) %x in format string (no. 25) requires 'unsigned int *' but the argument type is 'const unsigned int *'.\n"
                               "[test.cpp:29]: (warning) %x in format string (no. 28) requires 'unsigned int *' but the argument type is 'std::size_t * {aka unsigned long *}'.\n"
                               "[test.cpp:29]: (warning) %x in format string (no. 29) requires 'unsigned int *' but the argument type is 'std::ssize_t * {aka long *}'.\n"
                               "[test.cpp:29]: (warning) %x in format string (no. 30) requires 'unsigned int *' but the argument type is 'std::ptrdiff_t * {aka long *}'.\n"
                               "[test.cpp:29]: (warning) %x in format string (no. 31) requires 'unsigned int *' but the argument type is 'std::intptr_t * {aka long *}'.\n"
                               "[test.cpp:29]: (warning) %x in format string (no. 32) requires 'unsigned int *' but the argument type is 'std::uintptr_t * {aka unsigned long *}'.\n");
            std::string result_win64("[test.cpp:29]: (warning) %x in format string (no. 1) requires 'unsigned int *' but the argument type is 'bool *'.\n"
                                     "[test.cpp:29]: (warning) %x in format string (no. 2) requires 'unsigned int *' but the argument type is 'char *'.\n"
                                     "[test.cpp:29]: (warning) %x in format string (no. 3) requires 'unsigned int *' but the argument type is 'signed char *'.\n"
                                     "[test.cpp:29]: (warning) %x in format string (no. 4) requires 'unsigned int *' but the argument type is 'unsigned char *'.\n"
                                     "[test.cpp:29]: (warning) %x in format string (no. 5) requires 'unsigned int *' but the argument type is 'short *'.\n"
                                     "[test.cpp:29]: (warning) %x in format string (no. 6) requires 'unsigned int *' but the argument type is 'unsigned short *'.\n"
                                     "[test.cpp:29]: (warning) %x in format string (no. 9) requires 'unsigned int *' but the argument type is 'long *'.\n"
                                     "[test.cpp:29]: (warning) %x in format string (no. 10) requires 'unsigned int *' but the argument type is 'unsigned long *'.\n"
                                     "[test.cpp:29]: (warning) %x in format string (no. 11) requires 'unsigned int *' but the argument type is 'long long *'.\n"
                                     "[test.cpp:29]: (warning) %x in format string (no. 12) requires 'unsigned int *' but the argument type is 'unsigned long long *'.\n"
                                     "[test.cpp:29]: (warning) %x in format string (no. 13) requires 'unsigned int *' but the argument type is 'float *'.\n"
                                     "[test.cpp:29]: (warning) %x in format string (no. 14) requires 'unsigned int *' but the argument type is 'double *'.\n"
                                     "[test.cpp:29]: (warning) %x in format string (no. 15) requires 'unsigned int *' but the argument type is 'long double *'.\n"
                                     "[test.cpp:29]: (warning) %x in format string (no. 16) requires 'unsigned int *' but the argument type is 'size_t * {aka unsigned long long *}'.\n"
                                     "[test.cpp:29]: (warning) %x in format string (no. 17) requires 'unsigned int *' but the argument type is 'ssize_t * {aka long long *}'.\n"
                                     "[test.cpp:29]: (warning) %x in format string (no. 18) requires 'unsigned int *' but the argument type is 'ptrdiff_t * {aka long long *}'.\n"
                                     "[test.cpp:29]: (warning) %x in format string (no. 19) requires 'unsigned int *' but the argument type is 'intmax_t * {aka long long *}'.\n"
                                     "[test.cpp:29]: (warning) %x in format string (no. 20) requires 'unsigned int *' but the argument type is 'uintmax_t * {aka unsigned long long *}'.\n"
                                     "[test.cpp:29]: (warning) %x in format string (no. 21) requires 'unsigned int *' but the argument type is 'void * *'.\n"
                                     "[test.cpp:29]: (warning) %x in format string (no. 22) requires 'unsigned int *' but the argument type is 'void *'.\n"
                                     "[test.cpp:29]: (warning) %x in format string (no. 23) requires 'unsigned int *' but the argument type is 'const char *'.\n"
                                     "[test.cpp:29]: (warning) %x in format string (no. 24) requires 'unsigned int *' but the argument type is 'const wchar_t *'.\n"
                                     "[test.cpp:29]: (warning) %x in format string (no. 25) requires 'unsigned int *' but the argument type is 'const unsigned int *'.\n"
                                     "[test.cpp:29]: (warning) %x in format string (no. 28) requires 'unsigned int *' but the argument type is 'std::size_t * {aka unsigned long long *}'.\n"
                                     "[test.cpp:29]: (warning) %x in format string (no. 29) requires 'unsigned int *' but the argument type is 'std::ssize_t * {aka long long *}'.\n"
                                     "[test.cpp:29]: (warning) %x in format string (no. 30) requires 'unsigned int *' but the argument type is 'std::ptrdiff_t * {aka long long *}'.\n"
                                     "[test.cpp:29]: (warning) %x in format string (no. 31) requires 'unsigned int *' but the argument type is 'std::intptr_t * {aka long long *}'.\n"
                                     "[test.cpp:29]: (warning) %x in format string (no. 32) requires 'unsigned int *' but the argument type is 'std::uintptr_t * {aka unsigned long long *}'.\n");

            check(code, true, false, Settings::Unix32);
            ASSERT_EQUALS(result, errout.str());
            check(code, true, false, Settings::Unix64);
            ASSERT_EQUALS(result, errout.str());
            check(code, true, false, Settings::Win32A);
            ASSERT_EQUALS(result, errout.str());
            check(code, true, false, Settings::Win64);
            ASSERT_EQUALS(result_win64, errout.str());
        }

        {
            const char * code = "const float * foo() { }\n"
                                "void foo() {\n"
                                "    bool b;\n"
                                "    char c;\n"
                                "    signed char sc;\n"
                                "    unsigned char uc;\n"
                                "    short s;\n"
                                "    unsigned short us;\n"
                                "    int i;\n"
                                "    unsigned int ui;\n"
                                "    long l;\n"
                                "    unsigned long ul;\n"
                                "    long long ll;\n"
                                "    unsigned long long ull;\n"
                                "    float f;\n"
                                "    double d;\n"
                                "    long double ld;\n"
                                "    size_t st;\n"
                                "    ssize_t sst;\n"
                                "    ptrdiff_t pt;\n"
                                "    intmax_t it;\n"
                                "    uintmax_t ut;\n"
                                "    void * vp;\n"
                                "    std::size_t stdst;\n"
                                "    std::ssize_t stdsst;\n"
                                "    std::ptrdiff_t stdpt;\n"
                                "    std::intptr_t stdipt;\n"
                                "    std::uintptr_t stduipt;\n"
                                "    scanf(\"%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f\",\n"
                                "          &b, &c, &sc, &uc, &s, &us, &i, &ui, &l, &ul, &ll, &ull, &f, &d, &ld, &st, &sst, &pt, &it, &ut, &vp, vp,\n"
                                "          \"str\", L\"str\", foo(), &unknown, unknown, &stdst, &stdsst, &stdpt, &stdipt, &stduipt);\n"
                                "}\n";
            std::string result("[test.cpp:29]: (warning) %f in format string (no. 1) requires 'float *' but the argument type is 'bool *'.\n"
                               "[test.cpp:29]: (warning) %f in format string (no. 2) requires 'float *' but the argument type is 'char *'.\n"
                               "[test.cpp:29]: (warning) %f in format string (no. 3) requires 'float *' but the argument type is 'signed char *'.\n"
                               "[test.cpp:29]: (warning) %f in format string (no. 4) requires 'float *' but the argument type is 'unsigned char *'.\n"
                               "[test.cpp:29]: (warning) %f in format string (no. 5) requires 'float *' but the argument type is 'short *'.\n"
                               "[test.cpp:29]: (warning) %f in format string (no. 6) requires 'float *' but the argument type is 'unsigned short *'.\n"
                               "[test.cpp:29]: (warning) %f in format string (no. 7) requires 'float *' but the argument type is 'int *'.\n"
                               "[test.cpp:29]: (warning) %f in format string (no. 8) requires 'float *' but the argument type is 'unsigned int *'.\n"
                               "[test.cpp:29]: (warning) %f in format string (no. 9) requires 'float *' but the argument type is 'long *'.\n"
                               "[test.cpp:29]: (warning) %f in format string (no. 10) requires 'float *' but the argument type is 'unsigned long *'.\n"
                               "[test.cpp:29]: (warning) %f in format string (no. 11) requires 'float *' but the argument type is 'long long *'.\n"
                               "[test.cpp:29]: (warning) %f in format string (no. 12) requires 'float *' but the argument type is 'unsigned long long *'.\n"
                               "[test.cpp:29]: (warning) %f in format string (no. 14) requires 'float *' but the argument type is 'double *'.\n"
                               "[test.cpp:29]: (warning) %f in format string (no. 15) requires 'float *' but the argument type is 'long double *'.\n"
                               "[test.cpp:29]: (warning) %f in format string (no. 16) requires 'float *' but the argument type is 'size_t * {aka unsigned long *}'.\n"
                               "[test.cpp:29]: (warning) %f in format string (no. 17) requires 'float *' but the argument type is 'ssize_t * {aka long *}'.\n"
                               "[test.cpp:29]: (warning) %f in format string (no. 18) requires 'float *' but the argument type is 'ptrdiff_t * {aka long *}'.\n"
                               "[test.cpp:29]: (warning) %f in format string (no. 19) requires 'float *' but the argument type is 'intmax_t * {aka long *}'.\n"
                               "[test.cpp:29]: (warning) %f in format string (no. 20) requires 'float *' but the argument type is 'uintmax_t * {aka unsigned long *}'.\n"
                               "[test.cpp:29]: (warning) %f in format string (no. 21) requires 'float *' but the argument type is 'void * *'.\n"
                               "[test.cpp:29]: (warning) %f in format string (no. 22) requires 'float *' but the argument type is 'void *'.\n"
                               "[test.cpp:29]: (warning) %f in format string (no. 23) requires 'float *' but the argument type is 'const char *'.\n"
                               "[test.cpp:29]: (warning) %f in format string (no. 24) requires 'float *' but the argument type is 'const wchar_t *'.\n"
                               "[test.cpp:29]: (warning) %f in format string (no. 25) requires 'float *' but the argument type is 'const float *'.\n"
                               "[test.cpp:29]: (warning) %f in format string (no. 28) requires 'float *' but the argument type is 'std::size_t * {aka unsigned long *}'.\n"
                               "[test.cpp:29]: (warning) %f in format string (no. 29) requires 'float *' but the argument type is 'std::ssize_t * {aka long *}'.\n"
                               "[test.cpp:29]: (warning) %f in format string (no. 30) requires 'float *' but the argument type is 'std::ptrdiff_t * {aka long *}'.\n"
                               "[test.cpp:29]: (warning) %f in format string (no. 31) requires 'float *' but the argument type is 'std::intptr_t * {aka long *}'.\n"
                               "[test.cpp:29]: (warning) %f in format string (no. 32) requires 'float *' but the argument type is 'std::uintptr_t * {aka unsigned long *}'.\n");
            std::string result_win64("[test.cpp:29]: (warning) %f in format string (no. 1) requires 'float *' but the argument type is 'bool *'.\n"
                                     "[test.cpp:29]: (warning) %f in format string (no. 2) requires 'float *' but the argument type is 'char *'.\n"
                                     "[test.cpp:29]: (warning) %f in format string (no. 3) requires 'float *' but the argument type is 'signed char *'.\n"
                                     "[test.cpp:29]: (warning) %f in format string (no. 4) requires 'float *' but the argument type is 'unsigned char *'.\n"
                                     "[test.cpp:29]: (warning) %f in format string (no. 5) requires 'float *' but the argument type is 'short *'.\n"
                                     "[test.cpp:29]: (warning) %f in format string (no. 6) requires 'float *' but the argument type is 'unsigned short *'.\n"
                                     "[test.cpp:29]: (warning) %f in format string (no. 7) requires 'float *' but the argument type is 'int *'.\n"
                                     "[test.cpp:29]: (warning) %f in format string (no. 8) requires 'float *' but the argument type is 'unsigned int *'.\n"
                                     "[test.cpp:29]: (warning) %f in format string (no. 9) requires 'float *' but the argument type is 'long *'.\n"
                                     "[test.cpp:29]: (warning) %f in format string (no. 10) requires 'float *' but the argument type is 'unsigned long *'.\n"
                                     "[test.cpp:29]: (warning) %f in format string (no. 11) requires 'float *' but the argument type is 'long long *'.\n"
                                     "[test.cpp:29]: (warning) %f in format string (no. 12) requires 'float *' but the argument type is 'unsigned long long *'.\n"
                                     "[test.cpp:29]: (warning) %f in format string (no. 14) requires 'float *' but the argument type is 'double *'.\n"
                                     "[test.cpp:29]: (warning) %f in format string (no. 15) requires 'float *' but the argument type is 'long double *'.\n"
                                     "[test.cpp:29]: (warning) %f in format string (no. 16) requires 'float *' but the argument type is 'size_t * {aka unsigned long long *}'.\n"
                                     "[test.cpp:29]: (warning) %f in format string (no. 17) requires 'float *' but the argument type is 'ssize_t * {aka long long *}'.\n"
                                     "[test.cpp:29]: (warning) %f in format string (no. 18) requires 'float *' but the argument type is 'ptrdiff_t * {aka long long *}'.\n"
                                     "[test.cpp:29]: (warning) %f in format string (no. 19) requires 'float *' but the argument type is 'intmax_t * {aka long long *}'.\n"
                                     "[test.cpp:29]: (warning) %f in format string (no. 20) requires 'float *' but the argument type is 'uintmax_t * {aka unsigned long long *}'.\n"
                                     "[test.cpp:29]: (warning) %f in format string (no. 21) requires 'float *' but the argument type is 'void * *'.\n"
                                     "[test.cpp:29]: (warning) %f in format string (no. 22) requires 'float *' but the argument type is 'void *'.\n"
                                     "[test.cpp:29]: (warning) %f in format string (no. 23) requires 'float *' but the argument type is 'const char *'.\n"
                                     "[test.cpp:29]: (warning) %f in format string (no. 24) requires 'float *' but the argument type is 'const wchar_t *'.\n"
                                     "[test.cpp:29]: (warning) %f in format string (no. 25) requires 'float *' but the argument type is 'const float *'.\n"
                                     "[test.cpp:29]: (warning) %f in format string (no. 28) requires 'float *' but the argument type is 'std::size_t * {aka unsigned long long *}'.\n"
                                     "[test.cpp:29]: (warning) %f in format string (no. 29) requires 'float *' but the argument type is 'std::ssize_t * {aka long long *}'.\n"
                                     "[test.cpp:29]: (warning) %f in format string (no. 30) requires 'float *' but the argument type is 'std::ptrdiff_t * {aka long long *}'.\n"
                                     "[test.cpp:29]: (warning) %f in format string (no. 31) requires 'float *' but the argument type is 'std::intptr_t * {aka long long *}'.\n"
                                     "[test.cpp:29]: (warning) %f in format string (no. 32) requires 'float *' but the argument type is 'std::uintptr_t * {aka unsigned long long *}'.\n");

            check(code, true, false, Settings::Unix32);
            ASSERT_EQUALS(result, errout.str());
            check(code, true, false, Settings::Unix64);
            ASSERT_EQUALS(result, errout.str());
            check(code, true, false, Settings::Win32A);
            ASSERT_EQUALS(result, errout.str());
            check(code, true, false, Settings::Win64);
            ASSERT_EQUALS(result_win64, errout.str());
        }

        {
            const char * code = "const double * foo() { }\n"
                                "void foo() {\n"
                                "    bool b;\n"
                                "    char c;\n"
                                "    signed char sc;\n"
                                "    unsigned char uc;\n"
                                "    short s;\n"
                                "    unsigned short us;\n"
                                "    int i;\n"
                                "    unsigned int ui;\n"
                                "    long l;\n"
                                "    unsigned long ul;\n"
                                "    long long ll;\n"
                                "    unsigned long long ull;\n"
                                "    float f;\n"
                                "    double d;\n"
                                "    long double ld;\n"
                                "    size_t st;\n"
                                "    ssize_t sst;\n"
                                "    ptrdiff_t pt;\n"
                                "    intmax_t it;\n"
                                "    uintmax_t ut;\n"
                                "    void * vp;\n"
                                "    std::size_t stdst;\n"
                                "    std::ssize_t stdsst;\n"
                                "    std::ptrdiff_t stdpt;\n"
                                "    std::intptr_t stdipt;\n"
                                "    std::uintptr_t stduipt;\n"
                                "    scanf(\"%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf\",\n"
                                "          &b, &c, &sc, &uc, &s, &us, &i, &ui, &l, &ul, &ll, &ull, &f, &d, &ld, &st, &sst, &pt, &it, &ut, &vp, vp,\n"
                                "          \"str\", L\"str\", foo(), &unknown, unknown, &stdst, &stdsst, &stdpt, &stdipt, &stduipt);\n"
                                "}\n";
            std::string result("[test.cpp:29]: (warning) %lf in format string (no. 1) requires 'double *' but the argument type is 'bool *'.\n"
                               "[test.cpp:29]: (warning) %lf in format string (no. 2) requires 'double *' but the argument type is 'char *'.\n"
                               "[test.cpp:29]: (warning) %lf in format string (no. 3) requires 'double *' but the argument type is 'signed char *'.\n"
                               "[test.cpp:29]: (warning) %lf in format string (no. 4) requires 'double *' but the argument type is 'unsigned char *'.\n"
                               "[test.cpp:29]: (warning) %lf in format string (no. 5) requires 'double *' but the argument type is 'short *'.\n"
                               "[test.cpp:29]: (warning) %lf in format string (no. 6) requires 'double *' but the argument type is 'unsigned short *'.\n"
                               "[test.cpp:29]: (warning) %lf in format string (no. 7) requires 'double *' but the argument type is 'int *'.\n"
                               "[test.cpp:29]: (warning) %lf in format string (no. 8) requires 'double *' but the argument type is 'unsigned int *'.\n"
                               "[test.cpp:29]: (warning) %lf in format string (no. 9) requires 'double *' but the argument type is 'long *'.\n"
                               "[test.cpp:29]: (warning) %lf in format string (no. 10) requires 'double *' but the argument type is 'unsigned long *'.\n"
                               "[test.cpp:29]: (warning) %lf in format string (no. 11) requires 'double *' but the argument type is 'long long *'.\n"
                               "[test.cpp:29]: (warning) %lf in format string (no. 12) requires 'double *' but the argument type is 'unsigned long long *'.\n"
                               "[test.cpp:29]: (warning) %lf in format string (no. 13) requires 'double *' but the argument type is 'float *'.\n"
                               "[test.cpp:29]: (warning) %lf in format string (no. 15) requires 'double *' but the argument type is 'long double *'.\n"
                               "[test.cpp:29]: (warning) %lf in format string (no. 16) requires 'double *' but the argument type is 'size_t * {aka unsigned long *}'.\n"
                               "[test.cpp:29]: (warning) %lf in format string (no. 17) requires 'double *' but the argument type is 'ssize_t * {aka long *}'.\n"
                               "[test.cpp:29]: (warning) %lf in format string (no. 18) requires 'double *' but the argument type is 'ptrdiff_t * {aka long *}'.\n"
                               "[test.cpp:29]: (warning) %lf in format string (no. 19) requires 'double *' but the argument type is 'intmax_t * {aka long *}'.\n"
                               "[test.cpp:29]: (warning) %lf in format string (no. 20) requires 'double *' but the argument type is 'uintmax_t * {aka unsigned long *}'.\n"
                               "[test.cpp:29]: (warning) %lf in format string (no. 21) requires 'double *' but the argument type is 'void * *'.\n"
                               "[test.cpp:29]: (warning) %lf in format string (no. 22) requires 'double *' but the argument type is 'void *'.\n"
                               "[test.cpp:29]: (warning) %lf in format string (no. 23) requires 'double *' but the argument type is 'const char *'.\n"
                               "[test.cpp:29]: (warning) %lf in format string (no. 24) requires 'double *' but the argument type is 'const wchar_t *'.\n"
                               "[test.cpp:29]: (warning) %lf in format string (no. 25) requires 'double *' but the argument type is 'const double *'.\n"
                               "[test.cpp:29]: (warning) %lf in format string (no. 28) requires 'double *' but the argument type is 'std::size_t * {aka unsigned long *}'.\n"
                               "[test.cpp:29]: (warning) %lf in format string (no. 29) requires 'double *' but the argument type is 'std::ssize_t * {aka long *}'.\n"
                               "[test.cpp:29]: (warning) %lf in format string (no. 30) requires 'double *' but the argument type is 'std::ptrdiff_t * {aka long *}'.\n"
                               "[test.cpp:29]: (warning) %lf in format string (no. 31) requires 'double *' but the argument type is 'std::intptr_t * {aka long *}'.\n"
                               "[test.cpp:29]: (warning) %lf in format string (no. 32) requires 'double *' but the argument type is 'std::uintptr_t * {aka unsigned long *}'.\n");
            std::string result_win64("[test.cpp:29]: (warning) %lf in format string (no. 1) requires 'double *' but the argument type is 'bool *'.\n"
                                     "[test.cpp:29]: (warning) %lf in format string (no. 2) requires 'double *' but the argument type is 'char *'.\n"
                                     "[test.cpp:29]: (warning) %lf in format string (no. 3) requires 'double *' but the argument type is 'signed char *'.\n"
                                     "[test.cpp:29]: (warning) %lf in format string (no. 4) requires 'double *' but the argument type is 'unsigned char *'.\n"
                                     "[test.cpp:29]: (warning) %lf in format string (no. 5) requires 'double *' but the argument type is 'short *'.\n"
                                     "[test.cpp:29]: (warning) %lf in format string (no. 6) requires 'double *' but the argument type is 'unsigned short *'.\n"
                                     "[test.cpp:29]: (warning) %lf in format string (no. 7) requires 'double *' but the argument type is 'int *'.\n"
                                     "[test.cpp:29]: (warning) %lf in format string (no. 8) requires 'double *' but the argument type is 'unsigned int *'.\n"
                                     "[test.cpp:29]: (warning) %lf in format string (no. 9) requires 'double *' but the argument type is 'long *'.\n"
                                     "[test.cpp:29]: (warning) %lf in format string (no. 10) requires 'double *' but the argument type is 'unsigned long *'.\n"
                                     "[test.cpp:29]: (warning) %lf in format string (no. 11) requires 'double *' but the argument type is 'long long *'.\n"
                                     "[test.cpp:29]: (warning) %lf in format string (no. 12) requires 'double *' but the argument type is 'unsigned long long *'.\n"
                                     "[test.cpp:29]: (warning) %lf in format string (no. 13) requires 'double *' but the argument type is 'float *'.\n"
                                     "[test.cpp:29]: (warning) %lf in format string (no. 15) requires 'double *' but the argument type is 'long double *'.\n"
                                     "[test.cpp:29]: (warning) %lf in format string (no. 16) requires 'double *' but the argument type is 'size_t * {aka unsigned long long *}'.\n"
                                     "[test.cpp:29]: (warning) %lf in format string (no. 17) requires 'double *' but the argument type is 'ssize_t * {aka long long *}'.\n"
                                     "[test.cpp:29]: (warning) %lf in format string (no. 18) requires 'double *' but the argument type is 'ptrdiff_t * {aka long long *}'.\n"
                                     "[test.cpp:29]: (warning) %lf in format string (no. 19) requires 'double *' but the argument type is 'intmax_t * {aka long long *}'.\n"
                                     "[test.cpp:29]: (warning) %lf in format string (no. 20) requires 'double *' but the argument type is 'uintmax_t * {aka unsigned long long *}'.\n"
                                     "[test.cpp:29]: (warning) %lf in format string (no. 21) requires 'double *' but the argument type is 'void * *'.\n"
                                     "[test.cpp:29]: (warning) %lf in format string (no. 22) requires 'double *' but the argument type is 'void *'.\n"
                                     "[test.cpp:29]: (warning) %lf in format string (no. 23) requires 'double *' but the argument type is 'const char *'.\n"
                                     "[test.cpp:29]: (warning) %lf in format string (no. 24) requires 'double *' but the argument type is 'const wchar_t *'.\n"
                                     "[test.cpp:29]: (warning) %lf in format string (no. 25) requires 'double *' but the argument type is 'const double *'.\n"
                                     "[test.cpp:29]: (warning) %lf in format string (no. 28) requires 'double *' but the argument type is 'std::size_t * {aka unsigned long long *}'.\n"
                                     "[test.cpp:29]: (warning) %lf in format string (no. 29) requires 'double *' but the argument type is 'std::ssize_t * {aka long long *}'.\n"
                                     "[test.cpp:29]: (warning) %lf in format string (no. 30) requires 'double *' but the argument type is 'std::ptrdiff_t * {aka long long *}'.\n"
                                     "[test.cpp:29]: (warning) %lf in format string (no. 31) requires 'double *' but the argument type is 'std::intptr_t * {aka long long *}'.\n"
                                     "[test.cpp:29]: (warning) %lf in format string (no. 32) requires 'double *' but the argument type is 'std::uintptr_t * {aka unsigned long long *}'.\n");

            check(code, false, false, Settings::Unix32);
            ASSERT_EQUALS(result, errout.str());
            check(code, false, false, Settings::Unix64);
            ASSERT_EQUALS(result, errout.str());
            check(code, false, false, Settings::Win32A);
            ASSERT_EQUALS(result, errout.str());
            check(code, false, false, Settings::Win64);
            ASSERT_EQUALS(result_win64, errout.str());
        }

        {
            const char * code = "const long double * foo() { }\n"
                                "void foo() {\n"
                                "    bool b;\n"
                                "    char c;\n"
                                "    signed char sc;\n"
                                "    unsigned char uc;\n"
                                "    short s;\n"
                                "    unsigned short us;\n"
                                "    int i;\n"
                                "    unsigned int ui;\n"
                                "    long l;\n"
                                "    unsigned long ul;\n"
                                "    long long ll;\n"
                                "    unsigned long long ull;\n"
                                "    float f;\n"
                                "    double d;\n"
                                "    long double ld;\n"
                                "    size_t st;\n"
                                "    ssize_t sst;\n"
                                "    ptrdiff_t pt;\n"
                                "    intmax_t it;\n"
                                "    uintmax_t ut;\n"
                                "    void * vp;\n"
                                "    std::size_t stdst;\n"
                                "    std::ssize_t stdsst;\n"
                                "    std::ptrdiff_t stdpt;\n"
                                "    std::intptr_t stdipt;\n"
                                "    std::uintptr_t stduipt;\n"
                                "    scanf(\"%Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf %Lf\",\n"
                                "          &b, &c, &sc, &uc, &s, &us, &i, &ui, &l, &ul, &ll, &ull, &f, &d, &ld, &st, &sst, &pt, &it, &ut, &vp, vp,\n"
                                "          \"str\", L\"str\", foo(), &unknown, unknown, &stdst, &stdsst, &stdpt, &stdipt, &stduipt);\n"
                                "}\n";
            std::string result("[test.cpp:29]: (warning) %Lf in format string (no. 1) requires 'long double *' but the argument type is 'bool *'.\n"
                               "[test.cpp:29]: (warning) %Lf in format string (no. 2) requires 'long double *' but the argument type is 'char *'.\n"
                               "[test.cpp:29]: (warning) %Lf in format string (no. 3) requires 'long double *' but the argument type is 'signed char *'.\n"
                               "[test.cpp:29]: (warning) %Lf in format string (no. 4) requires 'long double *' but the argument type is 'unsigned char *'.\n"
                               "[test.cpp:29]: (warning) %Lf in format string (no. 5) requires 'long double *' but the argument type is 'short *'.\n"
                               "[test.cpp:29]: (warning) %Lf in format string (no. 6) requires 'long double *' but the argument type is 'unsigned short *'.\n"
                               "[test.cpp:29]: (warning) %Lf in format string (no. 7) requires 'long double *' but the argument type is 'int *'.\n"
                               "[test.cpp:29]: (warning) %Lf in format string (no. 8) requires 'long double *' but the argument type is 'unsigned int *'.\n"
                               "[test.cpp:29]: (warning) %Lf in format string (no. 9) requires 'long double *' but the argument type is 'long *'.\n"
                               "[test.cpp:29]: (warning) %Lf in format string (no. 10) requires 'long double *' but the argument type is 'unsigned long *'.\n"
                               "[test.cpp:29]: (warning) %Lf in format string (no. 11) requires 'long double *' but the argument type is 'long long *'.\n"
                               "[test.cpp:29]: (warning) %Lf in format string (no. 12) requires 'long double *' but the argument type is 'unsigned long long *'.\n"
                               "[test.cpp:29]: (warning) %Lf in format string (no. 13) requires 'long double *' but the argument type is 'float *'.\n"
                               "[test.cpp:29]: (warning) %Lf in format string (no. 14) requires 'long double *' but the argument type is 'double *'.\n"
                               "[test.cpp:29]: (warning) %Lf in format string (no. 16) requires 'long double *' but the argument type is 'size_t * {aka unsigned long *}'.\n"
                               "[test.cpp:29]: (warning) %Lf in format string (no. 17) requires 'long double *' but the argument type is 'ssize_t * {aka long *}'.\n"
                               "[test.cpp:29]: (warning) %Lf in format string (no. 18) requires 'long double *' but the argument type is 'ptrdiff_t * {aka long *}'.\n"
                               "[test.cpp:29]: (warning) %Lf in format string (no. 19) requires 'long double *' but the argument type is 'intmax_t * {aka long *}'.\n"
                               "[test.cpp:29]: (warning) %Lf in format string (no. 20) requires 'long double *' but the argument type is 'uintmax_t * {aka unsigned long *}'.\n"
                               "[test.cpp:29]: (warning) %Lf in format string (no. 21) requires 'long double *' but the argument type is 'void * *'.\n"
                               "[test.cpp:29]: (warning) %Lf in format string (no. 22) requires 'long double *' but the argument type is 'void *'.\n"
                               "[test.cpp:29]: (warning) %Lf in format string (no. 23) requires 'long double *' but the argument type is 'const char *'.\n"
                               "[test.cpp:29]: (warning) %Lf in format string (no. 24) requires 'long double *' but the argument type is 'const wchar_t *'.\n"
                               "[test.cpp:29]: (warning) %Lf in format string (no. 25) requires 'long double *' but the argument type is 'const long double *'.\n"
                               "[test.cpp:29]: (warning) %Lf in format string (no. 28) requires 'long double *' but the argument type is 'std::size_t * {aka unsigned long *}'.\n"
                               "[test.cpp:29]: (warning) %Lf in format string (no. 29) requires 'long double *' but the argument type is 'std::ssize_t * {aka long *}'.\n"
                               "[test.cpp:29]: (warning) %Lf in format string (no. 30) requires 'long double *' but the argument type is 'std::ptrdiff_t * {aka long *}'.\n"
                               "[test.cpp:29]: (warning) %Lf in format string (no. 31) requires 'long double *' but the argument type is 'std::intptr_t * {aka long *}'.\n"
                               "[test.cpp:29]: (warning) %Lf in format string (no. 32) requires 'long double *' but the argument type is 'std::uintptr_t * {aka unsigned long *}'.\n");
            std::string result_win64("[test.cpp:29]: (warning) %Lf in format string (no. 1) requires 'long double *' but the argument type is 'bool *'.\n"
                                     "[test.cpp:29]: (warning) %Lf in format string (no. 2) requires 'long double *' but the argument type is 'char *'.\n"
                                     "[test.cpp:29]: (warning) %Lf in format string (no. 3) requires 'long double *' but the argument type is 'signed char *'.\n"
                                     "[test.cpp:29]: (warning) %Lf in format string (no. 4) requires 'long double *' but the argument type is 'unsigned char *'.\n"
                                     "[test.cpp:29]: (warning) %Lf in format string (no. 5) requires 'long double *' but the argument type is 'short *'.\n"
                                     "[test.cpp:29]: (warning) %Lf in format string (no. 6) requires 'long double *' but the argument type is 'unsigned short *'.\n"
                                     "[test.cpp:29]: (warning) %Lf in format string (no. 7) requires 'long double *' but the argument type is 'int *'.\n"
                                     "[test.cpp:29]: (warning) %Lf in format string (no. 8) requires 'long double *' but the argument type is 'unsigned int *'.\n"
                                     "[test.cpp:29]: (warning) %Lf in format string (no. 9) requires 'long double *' but the argument type is 'long *'.\n"
                                     "[test.cpp:29]: (warning) %Lf in format string (no. 10) requires 'long double *' but the argument type is 'unsigned long *'.\n"
                                     "[test.cpp:29]: (warning) %Lf in format string (no. 11) requires 'long double *' but the argument type is 'long long *'.\n"
                                     "[test.cpp:29]: (warning) %Lf in format string (no. 12) requires 'long double *' but the argument type is 'unsigned long long *'.\n"
                                     "[test.cpp:29]: (warning) %Lf in format string (no. 13) requires 'long double *' but the argument type is 'float *'.\n"
                                     "[test.cpp:29]: (warning) %Lf in format string (no. 14) requires 'long double *' but the argument type is 'double *'.\n"
                                     "[test.cpp:29]: (warning) %Lf in format string (no. 16) requires 'long double *' but the argument type is 'size_t * {aka unsigned long long *}'.\n"
                                     "[test.cpp:29]: (warning) %Lf in format string (no. 17) requires 'long double *' but the argument type is 'ssize_t * {aka long long *}'.\n"
                                     "[test.cpp:29]: (warning) %Lf in format string (no. 18) requires 'long double *' but the argument type is 'ptrdiff_t * {aka long long *}'.\n"
                                     "[test.cpp:29]: (warning) %Lf in format string (no. 19) requires 'long double *' but the argument type is 'intmax_t * {aka long long *}'.\n"
                                     "[test.cpp:29]: (warning) %Lf in format string (no. 20) requires 'long double *' but the argument type is 'uintmax_t * {aka unsigned long long *}'.\n"
                                     "[test.cpp:29]: (warning) %Lf in format string (no. 21) requires 'long double *' but the argument type is 'void * *'.\n"
                                     "[test.cpp:29]: (warning) %Lf in format string (no. 22) requires 'long double *' but the argument type is 'void *'.\n"
                                     "[test.cpp:29]: (warning) %Lf in format string (no. 23) requires 'long double *' but the argument type is 'const char *'.\n"
                                     "[test.cpp:29]: (warning) %Lf in format string (no. 24) requires 'long double *' but the argument type is 'const wchar_t *'.\n"
                                     "[test.cpp:29]: (warning) %Lf in format string (no. 25) requires 'long double *' but the argument type is 'const long double *'.\n"
                                     "[test.cpp:29]: (warning) %Lf in format string (no. 28) requires 'long double *' but the argument type is 'std::size_t * {aka unsigned long long *}'.\n"
                                     "[test.cpp:29]: (warning) %Lf in format string (no. 29) requires 'long double *' but the argument type is 'std::ssize_t * {aka long long *}'.\n"
                                     "[test.cpp:29]: (warning) %Lf in format string (no. 30) requires 'long double *' but the argument type is 'std::ptrdiff_t * {aka long long *}'.\n"
                                     "[test.cpp:29]: (warning) %Lf in format string (no. 31) requires 'long double *' but the argument type is 'std::intptr_t * {aka long long *}'.\n"
                                     "[test.cpp:29]: (warning) %Lf in format string (no. 32) requires 'long double *' but the argument type is 'std::uintptr_t * {aka unsigned long long *}'.\n");

            check(code, false, false, Settings::Unix32);
            ASSERT_EQUALS(result, errout.str());
            check(code, false, false, Settings::Unix64);
            ASSERT_EQUALS(result, errout.str());
            check(code, false, false, Settings::Win32A);
            ASSERT_EQUALS(result, errout.str());
            check(code, false, false, Settings::Win64);
            ASSERT_EQUALS(result_win64, errout.str());
        }

        {
            const char * code = "const int * foo() { }\n"
                                "void foo() {\n"
                                "    bool b;\n"
                                "    char c;\n"
                                "    signed char sc;\n"
                                "    unsigned char uc;\n"
                                "    short s;\n"
                                "    unsigned short us;\n"
                                "    int i;\n"
                                "    unsigned int ui;\n"
                                "    long l;\n"
                                "    unsigned long ul;\n"
                                "    long long ll;\n"
                                "    unsigned long long ull;\n"
                                "    float f;\n"
                                "    double d;\n"
                                "    long double ld;\n"
                                "    size_t st;\n"
                                "    ssize_t sst;\n"
                                "    ptrdiff_t pt;\n"
                                "    intmax_t it;\n"
                                "    uintmax_t ut;\n"
                                "    void * vp;\n"
                                "    std::size_t stdst;\n"
                                "    std::ssize_t stdsst;\n"
                                "    std::ptrdiff_t stdpt;\n"
                                "    std::intptr_t stdipt;\n"
                                "    std::uintptr_t stduipt;\n"
                                "    scanf(\"%n %n %n %n %n %n %n %n %n %n %n %n %n %n %n %n %n %n %n %n %n %n %n %n %n %n %n %n %n %n %n %n\",\n"
                                "          &b, &c, &sc, &uc, &s, &us, &i, &ui, &l, &ul, &ll, &ull, &f, &d, &ld, &st, &sst, &pt, &it, &ut, &vp, vp,\n"
                                "          \"str\", L\"str\", foo(), &unknown, unknown, &stdst, &stdsst, &stdpt, &stdipt, &stduipt);\n"
                                "}\n";
            std::string result("[test.cpp:29]: (warning) %n in format string (no. 1) requires 'int *' but the argument type is 'bool *'.\n"
                               "[test.cpp:29]: (warning) %n in format string (no. 2) requires 'int *' but the argument type is 'char *'.\n"
                               "[test.cpp:29]: (warning) %n in format string (no. 3) requires 'int *' but the argument type is 'signed char *'.\n"
                               "[test.cpp:29]: (warning) %n in format string (no. 4) requires 'int *' but the argument type is 'unsigned char *'.\n"
                               "[test.cpp:29]: (warning) %n in format string (no. 5) requires 'int *' but the argument type is 'short *'.\n"
                               "[test.cpp:29]: (warning) %n in format string (no. 6) requires 'int *' but the argument type is 'unsigned short *'.\n"
                               "[test.cpp:29]: (warning) %n in format string (no. 8) requires 'int *' but the argument type is 'unsigned int *'.\n"
                               "[test.cpp:29]: (warning) %n in format string (no. 9) requires 'int *' but the argument type is 'long *'.\n"
                               "[test.cpp:29]: (warning) %n in format string (no. 10) requires 'int *' but the argument type is 'unsigned long *'.\n"
                               "[test.cpp:29]: (warning) %n in format string (no. 11) requires 'int *' but the argument type is 'long long *'.\n"
                               "[test.cpp:29]: (warning) %n in format string (no. 12) requires 'int *' but the argument type is 'unsigned long long *'.\n"
                               "[test.cpp:29]: (warning) %n in format string (no. 13) requires 'int *' but the argument type is 'float *'.\n"
                               "[test.cpp:29]: (warning) %n in format string (no. 14) requires 'int *' but the argument type is 'double *'.\n"
                               "[test.cpp:29]: (warning) %n in format string (no. 15) requires 'int *' but the argument type is 'long double *'.\n"
                               "[test.cpp:29]: (warning) %n in format string (no. 16) requires 'int *' but the argument type is 'size_t * {aka unsigned long *}'.\n"
                               "[test.cpp:29]: (warning) %n in format string (no. 17) requires 'int *' but the argument type is 'ssize_t * {aka long *}'.\n"
                               "[test.cpp:29]: (warning) %n in format string (no. 18) requires 'int *' but the argument type is 'ptrdiff_t * {aka long *}'.\n"
                               "[test.cpp:29]: (warning) %n in format string (no. 19) requires 'int *' but the argument type is 'intmax_t * {aka long *}'.\n"
                               "[test.cpp:29]: (warning) %n in format string (no. 20) requires 'int *' but the argument type is 'uintmax_t * {aka unsigned long *}'.\n"
                               "[test.cpp:29]: (warning) %n in format string (no. 21) requires 'int *' but the argument type is 'void * *'.\n"
                               "[test.cpp:29]: (warning) %n in format string (no. 22) requires 'int *' but the argument type is 'void *'.\n"
                               "[test.cpp:29]: (warning) %n in format string (no. 23) requires 'int *' but the argument type is 'const char *'.\n"
                               "[test.cpp:29]: (warning) %n in format string (no. 24) requires 'int *' but the argument type is 'const wchar_t *'.\n"
                               "[test.cpp:29]: (warning) %n in format string (no. 25) requires 'int *' but the argument type is 'const int *'.\n"
                               "[test.cpp:29]: (warning) %n in format string (no. 28) requires 'int *' but the argument type is 'std::size_t * {aka unsigned long *}'.\n"
                               "[test.cpp:29]: (warning) %n in format string (no. 29) requires 'int *' but the argument type is 'std::ssize_t * {aka long *}'.\n"
                               "[test.cpp:29]: (warning) %n in format string (no. 30) requires 'int *' but the argument type is 'std::ptrdiff_t * {aka long *}'.\n"
                               "[test.cpp:29]: (warning) %n in format string (no. 31) requires 'int *' but the argument type is 'std::intptr_t * {aka long *}'.\n"
                               "[test.cpp:29]: (warning) %n in format string (no. 32) requires 'int *' but the argument type is 'std::uintptr_t * {aka unsigned long *}'.\n");
            std::string result_win64("[test.cpp:29]: (warning) %n in format string (no. 1) requires 'int *' but the argument type is 'bool *'.\n"
                                     "[test.cpp:29]: (warning) %n in format string (no. 2) requires 'int *' but the argument type is 'char *'.\n"
                                     "[test.cpp:29]: (warning) %n in format string (no. 3) requires 'int *' but the argument type is 'signed char *'.\n"
                                     "[test.cpp:29]: (warning) %n in format string (no. 4) requires 'int *' but the argument type is 'unsigned char *'.\n"
                                     "[test.cpp:29]: (warning) %n in format string (no. 5) requires 'int *' but the argument type is 'short *'.\n"
                                     "[test.cpp:29]: (warning) %n in format string (no. 6) requires 'int *' but the argument type is 'unsigned short *'.\n"
                                     "[test.cpp:29]: (warning) %n in format string (no. 8) requires 'int *' but the argument type is 'unsigned int *'.\n"
                                     "[test.cpp:29]: (warning) %n in format string (no. 9) requires 'int *' but the argument type is 'long *'.\n"
                                     "[test.cpp:29]: (warning) %n in format string (no. 10) requires 'int *' but the argument type is 'unsigned long *'.\n"
                                     "[test.cpp:29]: (warning) %n in format string (no. 11) requires 'int *' but the argument type is 'long long *'.\n"
                                     "[test.cpp:29]: (warning) %n in format string (no. 12) requires 'int *' but the argument type is 'unsigned long long *'.\n"
                                     "[test.cpp:29]: (warning) %n in format string (no. 13) requires 'int *' but the argument type is 'float *'.\n"
                                     "[test.cpp:29]: (warning) %n in format string (no. 14) requires 'int *' but the argument type is 'double *'.\n"
                                     "[test.cpp:29]: (warning) %n in format string (no. 15) requires 'int *' but the argument type is 'long double *'.\n"
                                     "[test.cpp:29]: (warning) %n in format string (no. 16) requires 'int *' but the argument type is 'size_t * {aka unsigned long long *}'.\n"
                                     "[test.cpp:29]: (warning) %n in format string (no. 17) requires 'int *' but the argument type is 'ssize_t * {aka long long *}'.\n"
                                     "[test.cpp:29]: (warning) %n in format string (no. 18) requires 'int *' but the argument type is 'ptrdiff_t * {aka long long *}'.\n"
                                     "[test.cpp:29]: (warning) %n in format string (no. 19) requires 'int *' but the argument type is 'intmax_t * {aka long long *}'.\n"
                                     "[test.cpp:29]: (warning) %n in format string (no. 20) requires 'int *' but the argument type is 'uintmax_t * {aka unsigned long long *}'.\n"
                                     "[test.cpp:29]: (warning) %n in format string (no. 21) requires 'int *' but the argument type is 'void * *'.\n"
                                     "[test.cpp:29]: (warning) %n in format string (no. 22) requires 'int *' but the argument type is 'void *'.\n"
                                     "[test.cpp:29]: (warning) %n in format string (no. 23) requires 'int *' but the argument type is 'const char *'.\n"
                                     "[test.cpp:29]: (warning) %n in format string (no. 24) requires 'int *' but the argument type is 'const wchar_t *'.\n"
                                     "[test.cpp:29]: (warning) %n in format string (no. 25) requires 'int *' but the argument type is 'const int *'.\n"
                                     "[test.cpp:29]: (warning) %n in format string (no. 28) requires 'int *' but the argument type is 'std::size_t * {aka unsigned long long *}'.\n"
                                     "[test.cpp:29]: (warning) %n in format string (no. 29) requires 'int *' but the argument type is 'std::ssize_t * {aka long long *}'.\n"
                                     "[test.cpp:29]: (warning) %n in format string (no. 30) requires 'int *' but the argument type is 'std::ptrdiff_t * {aka long long *}'.\n"
                                     "[test.cpp:29]: (warning) %n in format string (no. 31) requires 'int *' but the argument type is 'std::intptr_t * {aka long long *}'.\n"
                                     "[test.cpp:29]: (warning) %n in format string (no. 32) requires 'int *' but the argument type is 'std::uintptr_t * {aka unsigned long long *}'.\n");

            check(code, false, false, Settings::Unix32);
            ASSERT_EQUALS(result, errout.str());
            check(code, false, false, Settings::Unix64);
            ASSERT_EQUALS(result, errout.str());
            check(code, false, false, Settings::Win32A);
            ASSERT_EQUALS(result, errout.str());
            check(code, false, false, Settings::Win64);
            ASSERT_EQUALS(result_win64, errout.str());
        }

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
            std::string result("[test.cpp:5]: (warning) %zd in format string (no. 1) requires 'ptrdiff_t *' but the argument type is 'size_t * {aka unsigned long *}'.\n");
            std::string result_win64("[test.cpp:5]: (warning) %zd in format string (no. 1) requires 'ptrdiff_t *' but the argument type is 'size_t * {aka unsigned long long *}'.\n");

            check(code, false, false, Settings::Unix32);
            ASSERT_EQUALS(result, errout.str());
            check(code, false, false, Settings::Unix64);
            ASSERT_EQUALS(result, errout.str());
            check(code, false, false, Settings::Win32A);
            ASSERT_EQUALS(result, errout.str());
            check(code, false, false, Settings::Win64);
            ASSERT_EQUALS(result_win64, errout.str());
        }
        {
            check("void g() {\n"
                  "    const char c[]=\"42\";\n"
                  "    scanf(\"%s\n\", c);\n"
                  "}\n");
            ASSERT_EQUALS("[test.cpp:3]: (warning) %s in format string (no. 1) requires a 'char *' but the argument type is 'const char *'.\n"
                          "[test.cpp:3]: (warning) scanf without field width limits can crash with huge input data.\n", errout.str());
        }

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
              "    printf(\"%u\", 123, bar());\n"
              "    printf(\"%u%s\", 0, bar(), 43123);\n"
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) printf format string requires 0 parameters but 1 is given.\n"
                      "[test.cpp:3]: (warning) printf format string requires 1 parameter but 2 are given.\n"
                      "[test.cpp:4]: (warning) printf format string requires 2 parameters but 3 are given.\n", errout.str());

        check("void foo() {\n" // swprintf exists as MSVC extension and as standard function: #4790
              "    swprintf(string1, L\"%u\", 32, string2);\n" // MSVC implementation
              "    swprintf(string1, L\"%s%s\", L\"a\", string2);\n" // MSVC implementation
              "    swprintf(string1, 6, L\"%u\", 32, string2);\n" // Standard implementation
              "    swprintf(string1, 6, L\"%u%s\", 32, string2);\n" // Standard implementation
              "}");
        ASSERT_EQUALS("[test.cpp:2]: (warning) swprintf format string requires 1 parameter but 2 are given.\n"
                      "[test.cpp:4]: (warning) swprintf format string requires 1 parameter but 2 are given.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:3]: (warning) %s in format string (no. 1) requires 'char *' but the argument type is 'int'.\n"
                      "[test.cpp:4]: (warning) %s in format string (no. 2) requires 'char *' but the argument type is 'int'.\n"
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
        ASSERT_EQUALS("[test.cpp:2]: (warning) %n in format string (no. 1) requires 'int *' but the argument type is 'const int *'.\n"
                      "[test.cpp:3]: (warning) %n in format string (no. 1) requires 'int *' but the argument type is 'const int'.\n"
                      "[test.cpp:4]: (warning) %n in format string (no. 1) requires 'int *' but the argument type is 'int'.\n"
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
                      "[test.cpp:6]: (warning) %x in format string (no. 1) requires 'unsigned int' but the argument type is 'const int *'.\n"
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
                      "[test.cpp:7]: (warning) %d in format string (no. 1) requires 'int' but the argument type is 'const int *'.\n"
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
                      "[test.cpp:6]: (warning) %u in format string (no. 1) requires 'unsigned int' but the argument type is 'int'.\n"
                      "[test.cpp:7]: (warning) %u in format string (no. 1) requires 'unsigned int' but the argument type is 'const int *'.\n"
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
                      "[test.cpp:5]: (warning) %f in format string (no. 1) requires 'double' but the argument type is 'const int *'.\n"
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
        ASSERT_EQUALS("", errout.str());

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
        ASSERT_EQUALS("[test.cpp:2]: (warning) %hd in format string (no. 1) requires 'int' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:3]: (warning) %hhd in format string (no. 1) requires 'int' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:4]: (warning) %ld in format string (no. 1) requires 'long' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:5]: (warning) %lld in format string (no. 1) requires 'long long' but the argument type is 'unsigned int'.\n" , errout.str());

        check("void foo(size_t s, ptrdiff_t p) {\n"
              "  printf(\"%zd\", s);\n"
              "  printf(\"%tu\", p);\n"
              "}", false, false, Settings::Unix32);
        ASSERT_EQUALS("[test.cpp:2]: (warning) %zd in format string (no. 1) requires 'ssize_t' but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:3]: (warning) %tu in format string (no. 1) requires 'unsigned ptrdiff_t' but the argument type is 'ptrdiff_t {aka long}'.\n", errout.str());

        check("void foo(std::size_t s, std::ptrdiff_t p) {\n"
              "  printf(\"%zd\", s);\n"
              "  printf(\"%tu\", p);\n"
              "}", false, false, Settings::Unix32);
        ASSERT_EQUALS("[test.cpp:2]: (warning) %zd in format string (no. 1) requires 'ssize_t' but the argument type is 'std::size_t {aka unsigned long}'.\n"
                      "[test.cpp:3]: (warning) %tu in format string (no. 1) requires 'unsigned ptrdiff_t' but the argument type is 'std::ptrdiff_t {aka long}'.\n", errout.str());

        check("void foo(size_t s, ptrdiff_t p) {\n"
              "  printf(\"%zd\", s);\n"
              "  printf(\"%tu\", p);\n"
              "}", false, false, Settings::Unix64);
        ASSERT_EQUALS("[test.cpp:2]: (warning) %zd in format string (no. 1) requires 'ssize_t' but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:3]: (warning) %tu in format string (no. 1) requires 'unsigned ptrdiff_t' but the argument type is 'ptrdiff_t {aka long}'.\n", errout.str());

        check("void foo(std::size_t s, std::ptrdiff_t p) {\n"
              "  printf(\"%zd\", s);\n"
              "  printf(\"%tu\", p);\n"
              "}", false, false, Settings::Unix64);
        ASSERT_EQUALS("[test.cpp:2]: (warning) %zd in format string (no. 1) requires 'ssize_t' but the argument type is 'std::size_t {aka unsigned long}'.\n"
                      "[test.cpp:3]: (warning) %tu in format string (no. 1) requires 'unsigned ptrdiff_t' but the argument type is 'std::ptrdiff_t {aka long}'.\n", errout.str());

        check("void foo(size_t s, ptrdiff_t p) {\n"
              "  printf(\"%zd\", s);\n"
              "  printf(\"%tu\", p);\n"
              "}", false, false, Settings::Win32A);
        ASSERT_EQUALS("[test.cpp:2]: (warning) %zd in format string (no. 1) requires 'ssize_t' but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:3]: (warning) %tu in format string (no. 1) requires 'unsigned ptrdiff_t' but the argument type is 'ptrdiff_t {aka long}'.\n", errout.str());

        check("void foo(std::size_t s, std::ptrdiff_t p) {\n"
              "  printf(\"%zd\", s);\n"
              "  printf(\"%tu\", p);\n"
              "}", false, false, Settings::Win32A);
        ASSERT_EQUALS("[test.cpp:2]: (warning) %zd in format string (no. 1) requires 'ssize_t' but the argument type is 'std::size_t {aka unsigned long}'.\n"
                      "[test.cpp:3]: (warning) %tu in format string (no. 1) requires 'unsigned ptrdiff_t' but the argument type is 'std::ptrdiff_t {aka long}'.\n", errout.str());

        check("void foo(size_t s, ptrdiff_t p) {\n"
              "  printf(\"%zd\", s);\n"
              "  printf(\"%tu\", p);\n"
              "}", false, false, Settings::Win64);
        ASSERT_EQUALS("[test.cpp:2]: (warning) %zd in format string (no. 1) requires 'ssize_t' but the argument type is 'size_t {aka unsigned long long}'.\n"
                      "[test.cpp:3]: (warning) %tu in format string (no. 1) requires 'unsigned ptrdiff_t' but the argument type is 'ptrdiff_t {aka long long}'.\n", errout.str());

        check("void foo(std::size_t s, std::ptrdiff_t p) {\n"
              "  printf(\"%zd\", s);\n"
              "  printf(\"%tu\", p);\n"
              "}", false, false, Settings::Win64);
        ASSERT_EQUALS("[test.cpp:2]: (warning) %zd in format string (no. 1) requires 'ssize_t' but the argument type is 'std::size_t {aka unsigned long long}'.\n"
                      "[test.cpp:3]: (warning) %tu in format string (no. 1) requires 'unsigned ptrdiff_t' but the argument type is 'std::ptrdiff_t {aka long long}'.\n", errout.str());

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
                      "[test.cpp:13]: (warning) %f in format string (no. 2) requires 'double' but the argument type is 'int'.\n"
                      "[test.cpp:13]: (warning) %f in format string (no. 3) requires 'double' but the argument type is 'int'.\n"
                      "[test.cpp:13]: (warning) %d in format string (no. 4) requires 'int' but the argument type is 'double'.\n"
                      "[test.cpp:13]: (warning) %f in format string (no. 5) requires 'double' but the argument type is 'int'.\n"
                      "[test.cpp:13]: (warning) %f in format string (no. 6) requires 'double' but the argument type is 'int'.\n", errout.str());

        check("short f() { return 0; }\n"
              "void foo() { printf(\"%d %u %lu %I64u %I64d %f %Lf %p\", f(), f(), f(), f(), f(), f(), f(), f()); }");
        ASSERT_EQUALS("[test.cpp:2]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'short'.\n"
                      "[test.cpp:2]: (warning) %lu in format string (no. 3) requires 'unsigned long' but the argument type is 'short'.\n"
                      "[test.cpp:2]: (warning) %I64u in format string (no. 4) requires 'unsigned __int64' but the argument type is 'short'.\n"
                      "[test.cpp:2]: (warning) %I64d in format string (no. 5) requires '__int64' but the argument type is 'short'.\n"
                      "[test.cpp:2]: (warning) %f in format string (no. 6) requires 'double' but the argument type is 'short'.\n"
                      "[test.cpp:2]: (warning) %Lf in format string (no. 7) requires 'long double' but the argument type is 'short'.\n"
                      "[test.cpp:2]: (warning) %p in format string (no. 8) requires an address but the argument type is 'short'.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:2]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'int'.\n"
                      "[test.cpp:2]: (warning) %lu in format string (no. 3) requires 'unsigned long' but the argument type is 'int'.\n"
                      "[test.cpp:2]: (warning) %I64u in format string (no. 4) requires 'unsigned __int64' but the argument type is 'int'.\n"
                      "[test.cpp:2]: (warning) %I64d in format string (no. 5) requires '__int64' but the argument type is 'int'.\n"
                      "[test.cpp:2]: (warning) %f in format string (no. 6) requires 'double' but the argument type is 'int'.\n"
                      "[test.cpp:2]: (warning) %Lf in format string (no. 7) requires 'long double' but the argument type is 'int'.\n"
                      "[test.cpp:2]: (warning) %p in format string (no. 8) requires an address but the argument type is 'int'.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:2]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'long'.\n"
                      "[test.cpp:2]: (warning) %lu in format string (no. 3) requires 'unsigned long' but the argument type is 'long'.\n"
                      "[test.cpp:2]: (warning) %I64u in format string (no. 4) requires 'unsigned __int64' but the argument type is 'long'.\n"
                      "[test.cpp:2]: (warning) %I64d in format string (no. 5) requires '__int64' but the argument type is 'long'.\n"
                      "[test.cpp:2]: (warning) %f in format string (no. 6) requires 'double' but the argument type is 'long'.\n"
                      "[test.cpp:2]: (warning) %Lf in format string (no. 7) requires 'long double' but the argument type is 'long'.\n"
                      "[test.cpp:2]: (warning) %p in format string (no. 8) requires an address but the argument type is 'long'.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:2]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'long long'.\n"
                      "[test.cpp:2]: (warning) %lu in format string (no. 3) requires 'unsigned long' but the argument type is 'long long'.\n"
                      "[test.cpp:2]: (warning) %I64u in format string (no. 4) requires 'unsigned __int64' but the argument type is 'long long'.\n"
                      "[test.cpp:2]: (warning) %f in format string (no. 6) requires 'double' but the argument type is 'long long'.\n"
                      "[test.cpp:2]: (warning) %Lf in format string (no. 7) requires 'long double' but the argument type is 'long long'.\n"
                      "[test.cpp:2]: (warning) %p in format string (no. 8) requires an address but the argument type is 'long long'.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:2]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'int'.\n"
                      "[test.cpp:2]: (warning) %lu in format string (no. 3) requires 'unsigned long' but the argument type is 'int'.\n"
                      "[test.cpp:2]: (warning) %f in format string (no. 4) requires 'double' but the argument type is 'int'.\n"
                      "[test.cpp:2]: (warning) %Lf in format string (no. 5) requires 'long double' but the argument type is 'int'.\n"
                      "[test.cpp:2]: (warning) %p in format string (no. 6) requires an address but the argument type is 'int'.\n", errout.str());

        check("struct Fred { int i; } f;\n"
              "void foo() { printf(\"%d %u %lu %f %Lf %p\", f.i, f.i, f.i, f.i, f.i, f.i); }");
        ASSERT_EQUALS("[test.cpp:2]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'int'.\n"
                      "[test.cpp:2]: (warning) %lu in format string (no. 3) requires 'unsigned long' but the argument type is 'int'.\n"
                      "[test.cpp:2]: (warning) %f in format string (no. 4) requires 'double' but the argument type is 'int'.\n"
                      "[test.cpp:2]: (warning) %Lf in format string (no. 5) requires 'long double' but the argument type is 'int'.\n"
                      "[test.cpp:2]: (warning) %p in format string (no. 6) requires an address but the argument type is 'int'.\n", errout.str());

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
        ASSERT_EQUALS("", errout.str());

        check("struct Fred { int i; } f;\n"
              "struct Fred & bar() { };\n"
              "void foo() { printf(\"%d %u %lu %f %Lf %p\", bar().i, bar().i, bar().i, bar().i, bar().i, bar().i); }");
        ASSERT_EQUALS("[test.cpp:3]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'int'.\n"
                      "[test.cpp:3]: (warning) %lu in format string (no. 3) requires 'unsigned long' but the argument type is 'int'.\n"
                      "[test.cpp:3]: (warning) %f in format string (no. 4) requires 'double' but the argument type is 'int'.\n"
                      "[test.cpp:3]: (warning) %Lf in format string (no. 5) requires 'long double' but the argument type is 'int'.\n"
                      "[test.cpp:3]: (warning) %p in format string (no. 6) requires an address but the argument type is 'int'.\n", errout.str());

        check("struct Fred { int i; } f;\n"
              "const struct Fred & bar() { };\n"
              "void foo() { printf(\"%d %u %lu %f %Lf %p\", bar().i, bar().i, bar().i, bar().i, bar().i, bar().i); }");
        ASSERT_EQUALS("[test.cpp:3]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'int'.\n"
                      "[test.cpp:3]: (warning) %lu in format string (no. 3) requires 'unsigned long' but the argument type is 'int'.\n"
                      "[test.cpp:3]: (warning) %f in format string (no. 4) requires 'double' but the argument type is 'int'.\n"
                      "[test.cpp:3]: (warning) %Lf in format string (no. 5) requires 'long double' but the argument type is 'int'.\n"
                      "[test.cpp:3]: (warning) %p in format string (no. 6) requires an address but the argument type is 'int'.\n", errout.str());

        check("struct Fred { int i; } f;\n"
              "static const struct Fred & bar() { };\n"
              "void foo() { printf(\"%d %u %lu %f %Lf %p\", bar().i, bar().i, bar().i, bar().i, bar().i, bar().i); }");
        ASSERT_EQUALS("[test.cpp:3]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'int'.\n"
                      "[test.cpp:3]: (warning) %lu in format string (no. 3) requires 'unsigned long' but the argument type is 'int'.\n"
                      "[test.cpp:3]: (warning) %f in format string (no. 4) requires 'double' but the argument type is 'int'.\n"
                      "[test.cpp:3]: (warning) %Lf in format string (no. 5) requires 'long double' but the argument type is 'int'.\n"
                      "[test.cpp:3]: (warning) %p in format string (no. 6) requires an address but the argument type is 'int'.\n", errout.str());

        check("struct Fred { int i; } f[2];\n"
              "struct Fred * bar() { return f; };\n"
              "void foo() { printf(\"%d %u %lu %f %Lf %p\", bar()[0].i, bar()[0].i, bar()[0].i, bar()[0].i, bar()[0].i, bar()[0].i); }");
        ASSERT_EQUALS("[test.cpp:3]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'int'.\n"
                      "[test.cpp:3]: (warning) %lu in format string (no. 3) requires 'unsigned long' but the argument type is 'int'.\n"
                      "[test.cpp:3]: (warning) %f in format string (no. 4) requires 'double' but the argument type is 'int'.\n"
                      "[test.cpp:3]: (warning) %Lf in format string (no. 5) requires 'long double' but the argument type is 'int'.\n"
                      "[test.cpp:3]: (warning) %p in format string (no. 6) requires an address but the argument type is 'int'.\n", errout.str());

        check("struct Fred { int i; } f[2];\n"
              "const struct Fred * bar() { return f; };\n"
              "void foo() { printf(\"%d %u %lu %f %Lf %p\", bar()[0].i, bar()[0].i, bar()[0].i, bar()[0].i, bar()[0].i, bar()[0].i); }");
        ASSERT_EQUALS("[test.cpp:3]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'int'.\n"
                      "[test.cpp:3]: (warning) %lu in format string (no. 3) requires 'unsigned long' but the argument type is 'int'.\n"
                      "[test.cpp:3]: (warning) %f in format string (no. 4) requires 'double' but the argument type is 'int'.\n"
                      "[test.cpp:3]: (warning) %Lf in format string (no. 5) requires 'long double' but the argument type is 'int'.\n"
                      "[test.cpp:3]: (warning) %p in format string (no. 6) requires an address but the argument type is 'int'.\n", errout.str());

        check("struct Fred { int i; } f[2];\n"
              "static const struct Fred * bar() { return f; };\n"
              "void foo() { printf(\"%d %u %lu %f %Lf %p\", bar()[0].i, bar()[0].i, bar()[0].i, bar()[0].i, bar()[0].i, bar()[0].i); }");
        ASSERT_EQUALS("[test.cpp:3]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'int'.\n"
                      "[test.cpp:3]: (warning) %lu in format string (no. 3) requires 'unsigned long' but the argument type is 'int'.\n"
                      "[test.cpp:3]: (warning) %f in format string (no. 4) requires 'double' but the argument type is 'int'.\n"
                      "[test.cpp:3]: (warning) %Lf in format string (no. 5) requires 'long double' but the argument type is 'int'.\n"
                      "[test.cpp:3]: (warning) %p in format string (no. 6) requires an address but the argument type is 'int'.\n", errout.str());

        check("struct Fred { int32_t i; } f;\n"
              "struct Fred & bar() { };\n"
              "void foo() { printf(\"%d %ld %u %lu %f %Lf\", bar().i, bar().i, bar().i, bar().i, bar().i, bar().i); }");
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
        ASSERT_EQUALS("[test.cpp:4]: (warning) %f in format string (no. 1) requires 'double' but the argument type is 'int'.\n", errout.str());

        check("struct Base { int length() { } };\n"
              "struct Derived : public Base { };\n"
              "void foo(Derived * d) {\n"
              "    printf(\"%f\", d.length());\n"
              "}\n");
        ASSERT_EQUALS("[test.cpp:4]: (warning) %f in format string (no. 1) requires 'double' but the argument type is 'int'.\n", errout.str());

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
              "}\n", false, false, Settings::Win32A);
        ASSERT_EQUALS("[test.cpp:4]: (warning) %d in format string (no. 3) requires 'int' but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:4]: (warning) %f in format string (no. 4) requires 'double' but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:5]: (warning) %d in format string (no. 3) requires 'int' but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:5]: (warning) %f in format string (no. 4) requires 'double' but the argument type is 'size_t {aka unsigned long}'.\n", errout.str());

        check("std::vector<int> v;\n"
              "std::string s;\n"
              "void foo() {\n"
              "    printf(\"%zu %Iu %d %f\", v.size(), v.size(), v.size(), v.size());\n"
              "    printf(\"%zu %Iu %d %f\", s.size(), s.size(), s.size(), s.size());\n"
              "}\n", false, false, Settings::Win64);
        ASSERT_EQUALS("[test.cpp:4]: (warning) %d in format string (no. 3) requires 'int' but the argument type is 'size_t {aka unsigned long long}'.\n"
                      "[test.cpp:4]: (warning) %f in format string (no. 4) requires 'double' but the argument type is 'size_t {aka unsigned long long}'.\n"
                      "[test.cpp:5]: (warning) %d in format string (no. 3) requires 'int' but the argument type is 'size_t {aka unsigned long long}'.\n"
                      "[test.cpp:5]: (warning) %f in format string (no. 4) requires 'double' but the argument type is 'size_t {aka unsigned long long}'.\n", errout.str());

        check("std::vector<int> v;\n"
              "std::string s;\n"
              "void foo() {\n"
              "    printf(\"%zu %Iu %d %f\", v.size(), v.size(), v.size(), v.size());\n"
              "    printf(\"%zu %Iu %d %f\", s.size(), s.size(), s.size(), s.size());\n"
              "}\n", false, false, Settings::Unix32);
        ASSERT_EQUALS("[test.cpp:4]: (warning) %d in format string (no. 3) requires 'int' but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:4]: (warning) %f in format string (no. 4) requires 'double' but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:5]: (warning) %d in format string (no. 3) requires 'int' but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:5]: (warning) %f in format string (no. 4) requires 'double' but the argument type is 'size_t {aka unsigned long}'.\n", errout.str());

        check("std::vector<int> v;\n"
              "std::string s;\n"
              "void foo() {\n"
              "    printf(\"%zu %Iu %d %f\", v.size(), v.size(), v.size(), v.size());\n"
              "    printf(\"%zu %Iu %d %f\", s.size(), s.size(), s.size(), s.size());\n"
              "}\n", false, false, Settings::Unix64);
        ASSERT_EQUALS("[test.cpp:4]: (warning) %d in format string (no. 3) requires 'int' but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:4]: (warning) %f in format string (no. 4) requires 'double' but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:5]: (warning) %d in format string (no. 3) requires 'int' but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:5]: (warning) %f in format string (no. 4) requires 'double' but the argument type is 'size_t {aka unsigned long}'.\n", errout.str());

        check("class Fred : public std::vector<int> {} v;\n"
              "std::string s;\n"
              "void foo() {\n"
              "    printf(\"%zu %Iu %d %f\", v.size(), v.size(), v.size(), v.size());\n"
              "    printf(\"%zu %Iu %d %f\", s.size(), s.size(), s.size(), s.size());\n"
              "}\n", false, false, Settings::Unix64);
        ASSERT_EQUALS("[test.cpp:4]: (warning) %d in format string (no. 3) requires 'int' but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:4]: (warning) %f in format string (no. 4) requires 'double' but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:5]: (warning) %d in format string (no. 3) requires 'int' but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:5]: (warning) %f in format string (no. 4) requires 'double' but the argument type is 'size_t {aka unsigned long}'.\n", errout.str());

        check("class Fred : public std::vector<int> {} v;\n"
              "void foo() {\n"
              "    printf(\"%d %u %f\", v[0], v[0], v[0]);\n"
              "}\n", false, false, Settings::Unix64);
        ASSERT_EQUALS("[test.cpp:3]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'int'.\n"
                      "[test.cpp:3]: (warning) %f in format string (no. 3) requires 'double' but the argument type is 'int'.\n", errout.str());

        check("std::string s;\n"
              "void foo() {\n"
              "    printf(\"%s %p %u %d %f\", s.c_str(), s.c_str(), s.c_str(), s.c_str(), s.c_str());\n"
              "}\n", false, false, Settings::Unix64);
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
              "}\n", false, false, Settings::Unix64);
        ASSERT_EQUALS("[test.cpp:8]: (warning) %u in format string (no. 1) requires 'unsigned int' but the argument type is 'char *'.\n"
                      "[test.cpp:8]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'char *'.\n"
                      "[test.cpp:8]: (warning) %u in format string (no. 3) requires 'unsigned int' but the argument type is 'char *'.\n"
                      "[test.cpp:9]: (warning) %u in format string (no. 1) requires 'unsigned int' but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:9]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:10]: (warning) %lu in format string (no. 1) requires 'unsigned long' but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:10]: (warning) %lu in format string (no. 2) requires 'unsigned long' but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:11]: (warning) %llu in format string (no. 1) requires 'unsigned long long' but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:11]: (warning) %llu in format string (no. 2) requires 'unsigned long long' but the argument type is 'size_t {aka unsigned long}'.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:13]: (warning) %zd in format string (no. 2) requires 'ssize_t' but the argument type is 'ptrdiff_t {aka long}'.\n"
                      "[test.cpp:13]: (warning) %d in format string (no. 9) requires 'int' but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:13]: (warning) %d in format string (no. 10) requires 'int' but the argument type is 'ptrdiff_t {aka long}'.\n"
                      "[test.cpp:13]: (warning) %d in format string (no. 11) requires 'int' but the argument type is 'char *'.\n"
                      "[test.cpp:13]: (warning) %d in format string (no. 12) requires 'int' but the argument type is 'char *'.\n"
                      "[test.cpp:13]: (warning) %d in format string (no. 13) requires 'int' but the argument type is 'char *'.\n"
                      "[test.cpp:14]: (warning) %ld in format string (no. 1) requires 'long' but the argument type is 'bool'.\n"
                      "[test.cpp:14]: (warning) %ld in format string (no. 2) requires 'long' but the argument type is 'char'.\n"
                      "[test.cpp:14]: (warning) %ld in format string (no. 3) requires 'long' but the argument type is 'signed char'.\n"
                      "[test.cpp:14]: (warning) %ld in format string (no. 4) requires 'long' but the argument type is 'unsigned char'.\n"
                      "[test.cpp:14]: (warning) %ld in format string (no. 5) requires 'long' but the argument type is 'short'.\n"
                      "[test.cpp:14]: (warning) %ld in format string (no. 6) requires 'long' but the argument type is 'unsigned short'.\n"
                      "[test.cpp:14]: (warning) %ld in format string (no. 7) requires 'long' but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:14]: (warning) %ld in format string (no. 8) requires 'long' but the argument type is 'ptrdiff_t {aka long}'.\n"
                      "[test.cpp:14]: (warning) %ld in format string (no. 9) requires 'long' but the argument type is 'char *'.\n"
                      "[test.cpp:14]: (warning) %ld in format string (no. 10) requires 'long' but the argument type is 'char *'.\n"
                      "[test.cpp:14]: (warning) %ld in format string (no. 11) requires 'long' but the argument type is 'char *'.\n"
                      "[test.cpp:15]: (warning) %zd in format string (no. 2) requires 'ssize_t' but the argument type is 'ptrdiff_t {aka long}'.\n"
                      "[test.cpp:15]: (warning) %d in format string (no. 9) requires 'int' but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:15]: (warning) %d in format string (no. 10) requires 'int' but the argument type is 'ptrdiff_t {aka long}'.\n"
                      "[test.cpp:15]: (warning) %d in format string (no. 11) requires 'int' but the argument type is 'char *'.\n"
                      "[test.cpp:16]: (warning) %ld in format string (no. 1) requires 'long' but the argument type is 'bool'.\n"
                      "[test.cpp:16]: (warning) %ld in format string (no. 2) requires 'long' but the argument type is 'char'.\n"
                      "[test.cpp:16]: (warning) %ld in format string (no. 3) requires 'long' but the argument type is 'signed char'.\n"
                      "[test.cpp:16]: (warning) %ld in format string (no. 4) requires 'long' but the argument type is 'unsigned char'.\n"
                      "[test.cpp:16]: (warning) %ld in format string (no. 5) requires 'long' but the argument type is 'short'.\n"
                      "[test.cpp:16]: (warning) %ld in format string (no. 6) requires 'long' but the argument type is 'unsigned short'.\n"
                      "[test.cpp:16]: (warning) %ld in format string (no. 7) requires 'long' but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:16]: (warning) %ld in format string (no. 8) requires 'long' but the argument type is 'ptrdiff_t {aka long}'.\n"
                      "[test.cpp:16]: (warning) %ld in format string (no. 9) requires 'long' but the argument type is 'char *'.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:6]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'size_t {aka unsigned long}'.\n"
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
              "}\n", false, false, Settings::Unix64);
        ASSERT_EQUALS("[test.cpp:7]: (warning) %u in format string (no. 1) requires 'unsigned int' but the argument type is 'size_t {aka unsigned long}'.\n", errout.str());

        check("DWORD a;\n"
              "DWORD_PTR b;\n"
              "void foo() {\n"
              "    printf(\"%u %u\", a, b);\n"
              "}\n", false, false, Settings::Win32A);
        ASSERT_EQUALS("[test.cpp:4]: (warning) %u in format string (no. 1) requires 'unsigned int' but the argument type is 'DWORD {aka unsigned long}'.\n"
                      "[test.cpp:4]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'DWORD_PTR {aka unsigned long}'.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:2]: (warning) %f in format string (no. 1) requires 'double' but the argument type is 'int'.\n"
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
        ASSERT_EQUALS("[test.cpp:3]: (warning) %d in format string (no. 1) requires 'int' but the argument type is 'long'.\n"
                      "[test.cpp:3]: (warning) %x in format string (no. 2) requires 'unsigned int' but the argument type is 'long'.\n"
                      "[test.cpp:3]: (warning) %u in format string (no. 3) requires 'unsigned int' but the argument type is 'long'.\n"
                      "[test.cpp:3]: (warning) %f in format string (no. 4) requires 'double' but the argument type is 'long'.\n", errout.str());

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
                      "[test.cpp:4]: (warning) %s in format string (no. 2) requires 'char *' but the argument type is 'int'.\n", errout.str());

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
              "}", false, false, Settings::Win32A);
        ASSERT_EQUALS("[test.cpp:8]: (warning) %Id in format string (no. 1) requires 'ptrdiff_t' but the argument type is 'size_t {aka unsigned long}'.\n"
                      "[test.cpp:9]: (warning) %Iu in format string (no. 2) requires 'size_t' but the argument type is 'ptrdiff_t {aka long}'.\n"
                      "[test.cpp:10]: (warning) %I32u in format string (no. 2) requires 'unsigned __int32' but the argument type is '__int32 {aka int}'.\n"
                      "[test.cpp:11]: (warning) %I32d in format string (no. 1) requires '__int32' but the argument type is 'unsigned __int32 {aka unsigned int}'.\n"
                      "[test.cpp:12]: (warning) %I64u in format string (no. 2) requires 'unsigned __int64' but the argument type is '__int64 {aka long long}'.\n"
                      "[test.cpp:13]: (warning) %I64d in format string (no. 1) requires '__int64' but the argument type is 'unsigned __int64 {aka unsigned long long}'.\n", errout.str());

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
        ASSERT_EQUALS("[test.cpp:8]: (warning) %Id in format string (no. 1) requires 'ptrdiff_t' but the argument type is 'size_t {aka unsigned long long}'.\n"
                      "[test.cpp:9]: (warning) %Iu in format string (no. 2) requires 'size_t' but the argument type is 'ptrdiff_t {aka long long}'.\n"
                      "[test.cpp:10]: (warning) %I32u in format string (no. 2) requires 'unsigned __int32' but the argument type is '__int32 {aka int}'.\n"
                      "[test.cpp:11]: (warning) %I32d in format string (no. 1) requires '__int32' but the argument type is 'unsigned __int32 {aka unsigned int}'.\n"
                      "[test.cpp:12]: (warning) %I64u in format string (no. 2) requires 'unsigned __int64' but the argument type is '__int64 {aka long long}'.\n"
                      "[test.cpp:13]: (warning) %I64d in format string (no. 1) requires '__int64' but the argument type is 'unsigned __int64 {aka unsigned long long}'.\n", errout.str());

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
              "}\n", false, false, Settings::Win64);
        ASSERT_EQUALS("", errout.str());

        check("void foo(LPARAM lp, WPARAM wp, LRESULT lr) {\n"
              "    printf(\"%Ix %Ix %Ix\", lp, wp, lr);\n"
              "}\n", false, false, Settings::Win32A);
        ASSERT_EQUALS("", errout.str());

        check("void foo(UINT32 a, ::UINT32 b, Fred::UINT32 c) {\n"
              "    printf(\"%d %d %d\n\", a, b, c);\n"
              "};\n", false, false, Settings::Win32A);
        ASSERT_EQUALS("[test.cpp:2]: (warning) %d in format string (no. 1) requires 'int' but the argument type is 'UINT32 {aka unsigned int}'.\n"
                      "[test.cpp:2]: (warning) %d in format string (no. 2) requires 'int' but the argument type is 'UINT32 {aka unsigned int}'.\n", errout.str());

        check("void foo(LPCVOID a, ::LPCVOID b, Fred::LPCVOID c) {\n"
              "    printf(\"%d %d %d\n\", a, b, c);\n"
              "};\n", false, false, Settings::Win32A);
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
              "}", false, false, Settings::Win32A);
        ASSERT_EQUALS("[test.cpp:8]: (warning) %Id in format string (no. 1) requires 'ptrdiff_t *' but the argument type is 'size_t * {aka unsigned long *}'.\n"
                      "[test.cpp:9]: (warning) %Iu in format string (no. 2) requires 'size_t *' but the argument type is 'ptrdiff_t * {aka long *}'.\n"
                      "[test.cpp:10]: (warning) %I32u in format string (no. 2) requires 'unsigned __int32 *' but the argument type is '__int32 * {aka int *}'.\n"
                      "[test.cpp:11]: (warning) %I32d in format string (no. 1) requires '__int32 *' but the argument type is 'unsigned __int32 * {aka unsigned int *}'.\n"
                      "[test.cpp:12]: (warning) %I64u in format string (no. 2) requires 'unsigned __int64 *' but the argument type is '__int64 * {aka long long *}'.\n"
                      "[test.cpp:13]: (warning) %I64d in format string (no. 1) requires '__int64 *' but the argument type is 'unsigned __int64 * {aka unsigned long long *}'.\n", errout.str());

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
              "}", false, false, Settings::Win64);
        ASSERT_EQUALS("[test.cpp:8]: (warning) %Id in format string (no. 1) requires 'ptrdiff_t *' but the argument type is 'size_t * {aka unsigned long long *}'.\n"
                      "[test.cpp:9]: (warning) %Iu in format string (no. 2) requires 'size_t *' but the argument type is 'ptrdiff_t * {aka long long *}'.\n"
                      "[test.cpp:10]: (warning) %I32u in format string (no. 2) requires 'unsigned __int32 *' but the argument type is '__int32 * {aka int *}'.\n"
                      "[test.cpp:11]: (warning) %I32d in format string (no. 1) requires '__int32 *' but the argument type is 'unsigned __int32 * {aka unsigned int *}'.\n"
                      "[test.cpp:12]: (warning) %I64u in format string (no. 2) requires 'unsigned __int64 *' but the argument type is '__int64 * {aka long long *}'.\n"
                      "[test.cpp:13]: (warning) %I64d in format string (no. 1) requires '__int64 *' but the argument type is 'unsigned __int64 * {aka unsigned long long *}'.\n", errout.str());

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
              "}", false, false, Settings::Win32A);
        ASSERT_EQUALS("", errout.str());

        check("void foo() {\n"
              "    unsigned __int32 u32;\n"
              "    CString string;\n"
              "    string.Format(\"%I32d\", u32);\n"
              "    string.AppendFormat(\"%I32d\", u32);\n"
              "}", false, false, Settings::Unix32);
        ASSERT_EQUALS("", errout.str());

        check("void foo() {\n"
              "    unsigned __int32 u32;\n"
              "    CString string;\n"
              "    string.Format(\"%I32d\", u32);\n"
              "    string.AppendFormat(\"%I32d\", u32);\n"
              "}", false, false, Settings::Win32A);
        ASSERT_EQUALS("[test.cpp:4]: (warning) %I32d in format string (no. 1) requires '__int32' but the argument type is 'unsigned __int32 {aka unsigned int}'.\n"
                      "[test.cpp:5]: (warning) %I32d in format string (no. 1) requires '__int32' but the argument type is 'unsigned __int32 {aka unsigned int}'.\n", errout.str());
    }

    void testMicrosoftSecurePrintfArgument() {
        check("void foo() {\n"
              "    int i;\n"
              "    unsigned int u;\n"
              "    _tprintf_s(_T(\"%d %u\"), u, i, 0);\n"
              "}\n", false, false, Settings::Win32A);
        ASSERT_EQUALS("[test.cpp:4]: (warning) %d in format string (no. 1) requires 'int' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:4]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'int'.\n"
                      "[test.cpp:4]: (warning) _tprintf_s format string requires 2 parameters but 3 are given.\n", errout.str());

        check("void foo() {\n"
              "    int i;\n"
              "    unsigned int u;\n"
              "    _tprintf_s(_T(\"%d %u\"), u, i, 0);\n"
              "}\n", false, false, Settings::Win32W);
        ASSERT_EQUALS("[test.cpp:4]: (warning) %d in format string (no. 1) requires 'int' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:4]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'int'.\n"
                      "[test.cpp:4]: (warning) _tprintf_s format string requires 2 parameters but 3 are given.\n", errout.str());

        check("void foo() {\n"
              "    int i;\n"
              "    unsigned int u;\n"
              "    printf_s(\"%d %u\", u, i, 0);\n"
              "}\n", false, false, Settings::Win32A);
        ASSERT_EQUALS("[test.cpp:4]: (warning) %d in format string (no. 1) requires 'int' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:4]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'int'.\n"
                      "[test.cpp:4]: (warning) printf_s format string requires 2 parameters but 3 are given.\n", errout.str());

        check("void foo() {\n"
              "    int i;\n"
              "    unsigned int u;\n"
              "    wprintf_s(L\"%d %u\", u, i, 0);\n"
              "}\n", false, false, Settings::Win32W);
        ASSERT_EQUALS("[test.cpp:4]: (warning) %d in format string (no. 1) requires 'int' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:4]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'int'.\n"
                      "[test.cpp:4]: (warning) wprintf_s format string requires 2 parameters but 3 are given.\n", errout.str());

        check("void foo() {\n"
              "    TCHAR str[10];\n"
              "    int i;\n"
              "    unsigned int u;\n"
              "    _stprintf_s(str, sizeof(str) / sizeof(TCHAR), _T(\"%d %u\"), u, i, 0);\n"
              "}\n", false, false, Settings::Win32A);
        ASSERT_EQUALS("[test.cpp:5]: (warning) %d in format string (no. 1) requires 'int' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:5]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'int'.\n"
                      "[test.cpp:5]: (warning) _stprintf_s format string requires 2 parameters but 3 are given.\n", errout.str());

        check("void foo() {\n"
              "    TCHAR str[10];\n"
              "    int i;\n"
              "    unsigned int u;\n"
              "    _stprintf_s(str, sizeof(str) / sizeof(TCHAR), _T(\"%d %u\"), u, i, 0);\n"
              "}\n", false, false, Settings::Win32W);
        ASSERT_EQUALS("[test.cpp:5]: (warning) %d in format string (no. 1) requires 'int' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:5]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'int'.\n"
                      "[test.cpp:5]: (warning) _stprintf_s format string requires 2 parameters but 3 are given.\n", errout.str());

        check("void foo() {\n"
              "    char str[10];\n"
              "    int i;\n"
              "    unsigned int u;\n"
              "    sprintf_s(str, sizeof(str), \"%d %u\", u, i, 0);\n"
              "}\n", false, false, Settings::Win32A);
        ASSERT_EQUALS("[test.cpp:5]: (warning) %d in format string (no. 1) requires 'int' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:5]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'int'.\n"
                      "[test.cpp:5]: (warning) sprintf_s format string requires 2 parameters but 3 are given.\n", errout.str());

        check("void foo() {\n"
              "    wchar_t str[10];\n"
              "    int i;\n"
              "    unsigned int u;\n"
              "    swprintf_s(str, sizeof(str) / sizeof(wchar_t), L\"%d %u\", u, i, 0);\n"
              "}\n", false, false, Settings::Win32W);
        ASSERT_EQUALS("[test.cpp:5]: (warning) %d in format string (no. 1) requires 'int' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:5]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'int'.\n"
                      "[test.cpp:5]: (warning) swprintf_s format string requires 2 parameters but 3 are given.\n", errout.str());

        check("void foo() {\n"
              "    TCHAR str[10];\n"
              "    int i;\n"
              "    unsigned int u;\n"
              "    _sntprintf_s(str, sizeof(str) / sizeof(TCHAR), _TRUNCATE, _T(\"%d %u\"), u, i, 0);\n"
              "}\n", false, false, Settings::Win32A);
        ASSERT_EQUALS("[test.cpp:5]: (warning) %d in format string (no. 1) requires 'int' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:5]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'int'.\n"
                      "[test.cpp:5]: (warning) _sntprintf_s format string requires 2 parameters but 3 are given.\n", errout.str());

        check("void foo() {\n"
              "    TCHAR str[10];\n"
              "    int i;\n"
              "    unsigned int u;\n"
              "    _sntprintf_s(str, sizeof(str) / sizeof(TCHAR), _TRUNCATE, _T(\"%d %u\"), u, i, 0);\n"
              "}\n", false, false, Settings::Win32W);
        ASSERT_EQUALS("[test.cpp:5]: (warning) %d in format string (no. 1) requires 'int' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:5]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'int'.\n"
                      "[test.cpp:5]: (warning) _sntprintf_s format string requires 2 parameters but 3 are given.\n", errout.str());

        check("void foo() {\n"
              "    char str[10];\n"
              "    int i;\n"
              "    unsigned int u;\n"
              "    _snprintf_s(str, sizeof(str), _TRUNCATE, \"%d %u\", u, i, 0);\n"
              "}\n", false, false, Settings::Win32A);
        ASSERT_EQUALS("[test.cpp:5]: (warning) %d in format string (no. 1) requires 'int' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:5]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'int'.\n"
                      "[test.cpp:5]: (warning) _snprintf_s format string requires 2 parameters but 3 are given.\n", errout.str());

        check("void foo() {\n"
              "    wchar_t str[10];\n"
              "    int i;\n"
              "    unsigned int u;\n"
              "    _snwprintf_s(str, sizeof(str) / sizeof(wchar_t), _TRUNCATE, L\"%d %u\", u, i, 0);\n"
              "}\n", false, false, Settings::Win32W);
        ASSERT_EQUALS("[test.cpp:5]: (warning) %d in format string (no. 1) requires 'int' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:5]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'int'.\n"
                      "[test.cpp:5]: (warning) _snwprintf_s format string requires 2 parameters but 3 are given.\n", errout.str());

        check("void foo(FILE * fp) {\n"
              "    int i;\n"
              "    unsigned int u;\n"
              "    _ftprintf_s(fp, _T(\"%d %u\"), u, i, 0);\n"
              "}\n", false, false, Settings::Win32A);
        ASSERT_EQUALS("[test.cpp:4]: (warning) %d in format string (no. 1) requires 'int' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:4]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'int'.\n"
                      "[test.cpp:4]: (warning) _ftprintf_s format string requires 2 parameters but 3 are given.\n", errout.str());

        check("void foo(FILE * fp) {\n"
              "    int i;\n"
              "    unsigned int u;\n"
              "    _ftprintf_s(fp, _T(\"%d %u\"), u, i, 0);\n"
              "}\n", false, false, Settings::Win32W);
        ASSERT_EQUALS("[test.cpp:4]: (warning) %d in format string (no. 1) requires 'int' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:4]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'int'.\n"
                      "[test.cpp:4]: (warning) _ftprintf_s format string requires 2 parameters but 3 are given.\n", errout.str());

        check("void foo(FILE * fp) {\n"
              "    int i;\n"
              "    unsigned int u;\n"
              "    fprintf_s(fp, \"%d %u\", u, i, 0);\n"
              "}\n", false, false, Settings::Win32A);
        ASSERT_EQUALS("[test.cpp:4]: (warning) %d in format string (no. 1) requires 'int' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:4]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'int'.\n"
                      "[test.cpp:4]: (warning) fprintf_s format string requires 2 parameters but 3 are given.\n", errout.str());

        check("void foo(FILE * fp) {\n"
              "    int i;\n"
              "    unsigned int u;\n"
              "    fwprintf_s(fp, L\"%d %u\", u, i, 0);\n"
              "}\n", false, false, Settings::Win32W);
        ASSERT_EQUALS("[test.cpp:4]: (warning) %d in format string (no. 1) requires 'int' but the argument type is 'unsigned int'.\n"
                      "[test.cpp:4]: (warning) %u in format string (no. 2) requires 'unsigned int' but the argument type is 'int'.\n"
                      "[test.cpp:4]: (warning) fwprintf_s format string requires 2 parameters but 3 are given.\n", errout.str());

        check("void foo() {\n"
              "    char lineBuffer [600];\n"
              "    const char * const format = \"%15s%17s%17s%17s%17s\n\";\n"
              "    sprintf_s(lineBuffer, format, \"type\", \"sum\", \"avg\", \"min\", \"max\");\n"
              "}\n", false, false, Settings::Win32A);
        ASSERT_EQUALS("", errout.str());

        check("void foo() {\n"
              "    const char * const format1 = \"%15s%17s%17s%17s%17s\n\";\n"
              "    const char format2[] = \"%15s%17s%17s%17s%17s\n\";\n"
              "    const char * const format3 = format1;\n" // we should warn about this someday
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
              "}\n", false, false, Settings::Win32A);
        ASSERT_EQUALS("[test.cpp:6]: (warning) %s in format string (no. 5) requires 'char *' but the argument type is 'int'.\n"
                      "[test.cpp:6]: (warning) sprintf_s format string requires 5 parameters but 6 are given.\n"
                      "[test.cpp:7]: (warning) %s in format string (no. 5) requires 'char *' but the argument type is 'int'.\n"
                      "[test.cpp:7]: (warning) sprintf_s format string requires 5 parameters but 6 are given.\n"
                      "[test.cpp:9]: (warning) %s in format string (no. 5) requires 'char *' but the argument type is 'int'.\n"
                      "[test.cpp:9]: (warning) sprintf format string requires 5 parameters but 6 are given.\n"
                      "[test.cpp:10]: (warning) %s in format string (no. 5) requires 'char *' but the argument type is 'int'.\n"
                      "[test.cpp:10]: (warning) sprintf format string requires 5 parameters but 6 are given.\n"
                      "[test.cpp:12]: (warning) %s in format string (no. 5) requires 'char *' but the argument type is 'int'.\n"
                      "[test.cpp:12]: (warning) printf format string requires 5 parameters but 6 are given.\n"
                      "[test.cpp:13]: (warning) %s in format string (no. 5) requires 'char *' but the argument type is 'int'.\n"
                      "[test.cpp:13]: (warning) printf format string requires 5 parameters but 6 are given.\n", errout.str());

    }

    void testMicrosoftSecureScanfArgument() {
        check("void foo() {\n"
              "    int i;\n"
              "    unsigned int u;\n"
              "    TCHAR str[10];\n"
              "    _tscanf_s(_T(\"%s %d %u %[a-z]\"), str, 10, &u, &i, str, 10, 0)\n"
              "}\n", false, false, Settings::Win32A);
        ASSERT_EQUALS("[test.cpp:5]: (warning) %d in format string (no. 2) requires 'int *' but the argument type is 'unsigned int *'.\n"
                      "[test.cpp:5]: (warning) %u in format string (no. 3) requires 'unsigned int *' but the argument type is 'int *'.\n"
                      "[test.cpp:5]: (warning) _tscanf_s format string requires 6 parameters but 7 are given.\n", errout.str());

        check("void foo() {\n"
              "    int i;\n"
              "    unsigned int u;\n"
              "    TCHAR str[10];\n"
              "    _tscanf_s(_T(\"%s %d %u %[a-z]\"), str, 10, &u, &i, str, 10, 0)\n"
              "}\n", false, false, Settings::Win32W);
        ASSERT_EQUALS("[test.cpp:5]: (warning) %d in format string (no. 2) requires 'int *' but the argument type is 'unsigned int *'.\n"
                      "[test.cpp:5]: (warning) %u in format string (no. 3) requires 'unsigned int *' but the argument type is 'int *'.\n"
                      "[test.cpp:5]: (warning) _tscanf_s format string requires 6 parameters but 7 are given.\n", errout.str());

        check("void foo() {\n"
              "    int i;\n"
              "    unsigned int u;\n"
              "    char str[10];\n"
              "    scanf_s(\"%s %d %u %[a-z]\", str, 10, &u, &i, str, 10, 0)\n"
              "}\n", false, false, Settings::Win32A);
        ASSERT_EQUALS("[test.cpp:5]: (warning) %d in format string (no. 2) requires 'int *' but the argument type is 'unsigned int *'.\n"
                      "[test.cpp:5]: (warning) %u in format string (no. 3) requires 'unsigned int *' but the argument type is 'int *'.\n"
                      "[test.cpp:5]: (warning) scanf_s format string requires 6 parameters but 7 are given.\n", errout.str());

        check("void foo() {\n"
              "    int i;\n"
              "    unsigned int u;\n"
              "    wchar_t str[10];\n"
              "    wscanf_s(L\"%s %d %u %[a-z]\", str, 10, &u, &i, str, 10, 0)\n"
              "}\n", false, false, Settings::Win32W);
        ASSERT_EQUALS("[test.cpp:5]: (warning) %d in format string (no. 2) requires 'int *' but the argument type is 'unsigned int *'.\n"
                      "[test.cpp:5]: (warning) %u in format string (no. 3) requires 'unsigned int *' but the argument type is 'int *'.\n"
                      "[test.cpp:5]: (warning) wscanf_s format string requires 6 parameters but 7 are given.\n", errout.str());

        check("void foo() {\n"
              "    TCHAR txt[100];\n"
              "    int i;\n"
              "    unsigned int u;\n"
              "    TCHAR str[10];\n"
              "    _stscanf_s(txt, _T(\"%s %d %u %[a-z]\"), str, 10, &u, &i, str, 10, 0)\n"
              "}\n", false, false, Settings::Win32A);
        ASSERT_EQUALS("[test.cpp:6]: (warning) %d in format string (no. 2) requires 'int *' but the argument type is 'unsigned int *'.\n"
                      "[test.cpp:6]: (warning) %u in format string (no. 3) requires 'unsigned int *' but the argument type is 'int *'.\n"
                      "[test.cpp:6]: (warning) _stscanf_s format string requires 6 parameters but 7 are given.\n", errout.str());

        check("void foo() {\n"
              "    TCHAR txt[100];\n"
              "    int i;\n"
              "    unsigned int u;\n"
              "    TCHAR str[10];\n"
              "    _stscanf_s(txt, _T(\"%s %d %u %[a-z]\"), str, 10, &u, &i, str, 10, 0)\n"
              "}\n", false, false, Settings::Win32W);
        ASSERT_EQUALS("[test.cpp:6]: (warning) %d in format string (no. 2) requires 'int *' but the argument type is 'unsigned int *'.\n"
                      "[test.cpp:6]: (warning) %u in format string (no. 3) requires 'unsigned int *' but the argument type is 'int *'.\n"
                      "[test.cpp:6]: (warning) _stscanf_s format string requires 6 parameters but 7 are given.\n", errout.str());

        check("void foo() {\n"
              "    char txt[100];\n"
              "    int i;\n"
              "    unsigned int u;\n"
              "    char str[10];\n"
              "    sscanf_s(txt, \"%s %d %u %[a-z]\", str, 10, &u, &i, str, 10, 0)\n"
              "}\n", false, false, Settings::Win32A);
        ASSERT_EQUALS("[test.cpp:6]: (warning) %d in format string (no. 2) requires 'int *' but the argument type is 'unsigned int *'.\n"
                      "[test.cpp:6]: (warning) %u in format string (no. 3) requires 'unsigned int *' but the argument type is 'int *'.\n"
                      "[test.cpp:6]: (warning) sscanf_s format string requires 6 parameters but 7 are given.\n", errout.str());

        check("void foo() {\n"
              "    wchar_t txt[100];\n"
              "    int i;\n"
              "    unsigned int u;\n"
              "    wchar_t str[10];\n"
              "    swscanf_s(txt, L\"%s %d %u %[a-z]\", str, 10, &u, &i, str, 10, 0)\n"
              "}\n", false, false, Settings::Win32W);
        ASSERT_EQUALS("[test.cpp:6]: (warning) %d in format string (no. 2) requires 'int *' but the argument type is 'unsigned int *'.\n"
                      "[test.cpp:6]: (warning) %u in format string (no. 3) requires 'unsigned int *' but the argument type is 'int *'.\n"
                      "[test.cpp:6]: (warning) swscanf_s format string requires 6 parameters but 7 are given.\n", errout.str());

        check("void foo(FILE * fp) {\n"
              "    int i;\n"
              "    unsigned int u;\n"
              "    TCHAR str[10];\n"
              "    _ftscanf_s(fp, _T(\"%s %d %u %[a-z]\"), str, 10, &u, &i, str, 10, 0)\n"
              "}\n", false, false, Settings::Win32A);
        ASSERT_EQUALS("[test.cpp:5]: (warning) %d in format string (no. 2) requires 'int *' but the argument type is 'unsigned int *'.\n"
                      "[test.cpp:5]: (warning) %u in format string (no. 3) requires 'unsigned int *' but the argument type is 'int *'.\n"
                      "[test.cpp:5]: (warning) _ftscanf_s format string requires 6 parameters but 7 are given.\n", errout.str());

        check("void foo(FILE * fp) {\n"
              "    int i;\n"
              "    unsigned int u;\n"
              "    TCHAR str[10];\n"
              "    _ftscanf_s(fp, _T(\"%s %d %u %[a-z]\"), str, 10, &u, &i, str, 10, 0)\n"
              "}\n", false, false, Settings::Win32W);
        ASSERT_EQUALS("[test.cpp:5]: (warning) %d in format string (no. 2) requires 'int *' but the argument type is 'unsigned int *'.\n"
                      "[test.cpp:5]: (warning) %u in format string (no. 3) requires 'unsigned int *' but the argument type is 'int *'.\n"
                      "[test.cpp:5]: (warning) _ftscanf_s format string requires 6 parameters but 7 are given.\n", errout.str());

        check("void foo(FILE * fp) {\n"
              "    int i;\n"
              "    unsigned int u;\n"
              "    char str[10];\n"
              "    fscanf_s(fp, \"%s %d %u %[a-z]\", str, 10, &u, &i, str, 10, 0)\n"
              "}\n", false, false, Settings::Win32A);
        ASSERT_EQUALS("[test.cpp:5]: (warning) %d in format string (no. 2) requires 'int *' but the argument type is 'unsigned int *'.\n"
                      "[test.cpp:5]: (warning) %u in format string (no. 3) requires 'unsigned int *' but the argument type is 'int *'.\n"
                      "[test.cpp:5]: (warning) fscanf_s format string requires 6 parameters but 7 are given.\n", errout.str());

        check("void foo(FILE * fp) {\n"
              "    int i;\n"
              "    unsigned int u;\n"
              "    wchar_t str[10];\n"
              "    fwscanf_s(fp, L\"%s %d %u %[a-z]\", str, 10, &u, &i, str, 10, 0)\n"
              "}\n", false, false, Settings::Win32W);
        ASSERT_EQUALS("[test.cpp:5]: (warning) %d in format string (no. 2) requires 'int *' but the argument type is 'unsigned int *'.\n"
                      "[test.cpp:5]: (warning) %u in format string (no. 3) requires 'unsigned int *' but the argument type is 'int *'.\n"
                      "[test.cpp:5]: (warning) fwscanf_s format string requires 6 parameters but 7 are given.\n", errout.str());

        check("void foo() {\n"
              "    WCHAR msStr1[5] = {0};\n"
              "    wscanf_s(L\"%4[^-]\", msStr1, _countof(msStr1));\n"
              "}\n", false, false, Settings::Win32W);
        ASSERT_EQUALS("", errout.str());
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

};

REGISTER_TEST(TestIO)
