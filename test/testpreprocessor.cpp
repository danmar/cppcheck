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


// The preprocessor that Cppcheck uses is a bit special. Instead of generating
// the code for a known configuration, it generates the code for each configuration.


#include "testsuite.h"
#include "preprocessor.h"
#include "tokenize.h"
#include "token.h"
#include "settings.h"

#include <map>
#include <string>
#include <set>


#ifdef _MSC_VER
// Visual Studio complains about truncated values for '(char)0xff' and '(char)0xfe'
// TODO: Is there any nice way to fix these warnings?
#pragma warning( disable : 4310 )
#endif

class TestPreprocessor : public TestFixture {
public:
    TestPreprocessor() : TestFixture("TestPreprocessor") {
        Preprocessor::macroChar = '$';
    }

    class OurPreprocessor : public Preprocessor {
    public:
        static std::string replaceIfDefined(const std::string &str) {
            Preprocessor p;
            return p.replaceIfDefined(str);
        }

        static std::string expandMacros(const std::string& code, ErrorLogger *errorLogger = 0) {
            return Preprocessor::expandMacros(code, "file.cpp", "", errorLogger);
        }

        static int getHeaderFileName(std::string &str) {
            return Preprocessor::getHeaderFileName(str);
        }
    };

private:

    void run() {
        // Just read the code into a string. Perform simple cleanup of the code
        TEST_CASE(readCode1);
        TEST_CASE(readCode2); // #4308 - convert C++11 raw string to plain old C string
        TEST_CASE(readCode3);
        TEST_CASE(readCode4); // #4351 - escaped whitespace in gcc

        // reading utf-16 file
        TEST_CASE(utf16);

        // remove comments
        TEST_CASE(removeComments);

        // The bug that started the whole work with the new preprocessor
        TEST_CASE(Bug2190219);

        TEST_CASE(test1);
        TEST_CASE(test2);
        TEST_CASE(test3);
        TEST_CASE(test4);
        TEST_CASE(test5);
        TEST_CASE(test6);
        TEST_CASE(test7);
        TEST_CASE(test7a);
        TEST_CASE(test7b);
        TEST_CASE(test7c);
        TEST_CASE(test7d);
        TEST_CASE(test7e);
        TEST_CASE(test8);  // #if A==1  => cfg: A=1
        TEST_CASE(test9);  // Don't crash for invalid code
        TEST_CASE(test10); // Ticket #5139


        TEST_CASE(error1); // #error => don't extract any code
        TEST_CASE(error2); // #error with extended chars
        TEST_CASE(error3);
        TEST_CASE(error4);  // #2919 - wrong filename is reported
        TEST_CASE(error5);

        TEST_CASE(if0_exclude);
        TEST_CASE(if0_whitespace);
        TEST_CASE(if0_else);
        TEST_CASE(if0_elif);

        // Don't handle include in a #if 0 block
        TEST_CASE(if0_include_1);
        TEST_CASE(if0_include_2);

        // Handling include guards (don't create extra configuration for it)
        TEST_CASE(includeguard1);
        TEST_CASE(includeguard2);

        TEST_CASE(newlines);

        TEST_CASE(comments1);

        TEST_CASE(if0);
        TEST_CASE(if1);

        TEST_CASE(elif);

        // Test the Preprocessor::match_cfg_def
        TEST_CASE(match_cfg_def);

        TEST_CASE(if_cond1);
        TEST_CASE(if_cond2);
        TEST_CASE(if_cond3);
        TEST_CASE(if_cond4);
        TEST_CASE(if_cond5);
        TEST_CASE(if_cond6);
        TEST_CASE(if_cond8);
        TEST_CASE(if_cond9);
        TEST_CASE(if_cond10);
        TEST_CASE(if_cond11);
        TEST_CASE(if_cond12);
        TEST_CASE(if_cond13);
        TEST_CASE(if_cond14);
        TEST_CASE(if_cond15); // #4456 - segfault

        TEST_CASE(if_or_1);
        TEST_CASE(if_or_2);

        TEST_CASE(if_macro_eq_macro); // #3536
        TEST_CASE(ticket_3675);
        TEST_CASE(ticket_3699);
        TEST_CASE(ticket_4922); // #4922

        TEST_CASE(multiline1);
        TEST_CASE(multiline2);
        TEST_CASE(multiline3);
        TEST_CASE(multiline4);
        TEST_CASE(multiline5);

        TEST_CASE(remove_asm);

        TEST_CASE(if_defined);      // "#if defined(AAA)" => "#ifdef AAA"
        TEST_CASE(if_not_defined);  // "#if !defined(AAA)" => "#ifndef AAA"

        // Macros..
        TEST_CASE(macro_simple1);
        TEST_CASE(macro_simple2);
        TEST_CASE(macro_simple3);
        TEST_CASE(macro_simple4);
        TEST_CASE(macro_simple5);
        TEST_CASE(macro_simple6);
        TEST_CASE(macro_simple7);
        TEST_CASE(macro_simple8);
        TEST_CASE(macro_simple9);
        TEST_CASE(macro_simple10);
        TEST_CASE(macro_simple11);
        TEST_CASE(macro_simple12);
        TEST_CASE(macro_simple13);
        TEST_CASE(macro_simple14);
        TEST_CASE(macro_simple15);
        TEST_CASE(macro_simple16);  // #4703: Macro parameters not trimmed
        TEST_CASE(macro_simple17);  // #5074: isExpandedMacro not set
        TEST_CASE(macro_simple18);  // (1e-7)
        TEST_CASE(macroInMacro1);
        TEST_CASE(macroInMacro2);
        TEST_CASE(macro_mismatch);
        TEST_CASE(macro_linenumbers);
        TEST_CASE(macro_nopar);
        TEST_CASE(macro_switchCase);
        TEST_CASE(macro_NULL); // skip #define NULL .. it is replaced in the tokenizer
        TEST_CASE(string1);
        TEST_CASE(string2);
        TEST_CASE(string3);
        TEST_CASE(preprocessor_undef);
        TEST_CASE(defdef);  // Defined multiple times
        TEST_CASE(preprocessor_doublesharp);
        TEST_CASE(preprocessor_include_in_str);
        TEST_CASE(va_args_1);
        TEST_CASE(va_args_2);
        TEST_CASE(va_args_3);
        TEST_CASE(va_args_4);
        TEST_CASE(multi_character_character);

        TEST_CASE(stringify);
        TEST_CASE(stringify2);
        TEST_CASE(stringify3);
        TEST_CASE(stringify4);
        TEST_CASE(stringify5);
        TEST_CASE(ifdefwithfile);
        TEST_CASE(pragma);
        TEST_CASE(pragma_asm_1);
        TEST_CASE(pragma_asm_2);
        TEST_CASE(pragma_once);
        TEST_CASE(endifsemicolon);
        TEST_CASE(missing_doublequote);
        TEST_CASE(handle_error);
        TEST_CASE(dup_defines);

        TEST_CASE(unicodeInCode);
        TEST_CASE(unicodeInComment);
        TEST_CASE(unicodeInString);
        TEST_CASE(define_part_of_func);
        TEST_CASE(conditionalDefine);
        TEST_CASE(multiline_comment);
        TEST_CASE(macro_parameters);
        TEST_CASE(newline_in_macro);
        TEST_CASE(includes);
        TEST_CASE(ifdef_ifdefined);

        // define and then ifdef
        TEST_CASE(define_if1);
        TEST_CASE(define_if2);
        TEST_CASE(define_if3);
        TEST_CASE(define_if4); // #4079 - #define X +123
        TEST_CASE(define_if5); // #4516 - #define B (A & 0x00f0)
        TEST_CASE(define_if6); // #4863 - #define B (A?-1:1)
        TEST_CASE(define_ifdef);
        TEST_CASE(define_ifndef1);
        TEST_CASE(define_ifndef2);
        TEST_CASE(ifndef_define);
        TEST_CASE(undef_ifdef);
        TEST_CASE(endfile);

        TEST_CASE(redundant_config);

        TEST_CASE(testPreprocessorRead1);
        TEST_CASE(testPreprocessorRead2);
        TEST_CASE(testPreprocessorRead3);
        TEST_CASE(testPreprocessorRead4);

        TEST_CASE(invalid_define_1); // #2605 - hang for: '#define ='
        TEST_CASE(invalid_define_2); // #4036 - hang for: '#define () {(int f(x) }'

        // Show 'missing include' warnings
        TEST_CASE(missingInclude);

        // inline suppression, missingInclude
        TEST_CASE(inline_suppression_for_missing_include);

        // Using -D to predefine symbols
        TEST_CASE(predefine1);
        TEST_CASE(predefine2);
        TEST_CASE(predefine3);
        TEST_CASE(predefine4);
        TEST_CASE(predefine5);  // automatically define __cplusplus
        TEST_CASE(predefine6);  // using -D and -f => check all matching configurations

        // Test Preprocessor::simplifyCondition
        TEST_CASE(simplifyCondition);
        TEST_CASE(invalidElIf); // #2942 segfault

        // Defines are given: test Preprocessor::handleIncludes
        TEST_CASE(def_handleIncludes);
        TEST_CASE(def_missingInclude);
        TEST_CASE(def_handleIncludes_ifelse1);   // problems in handleIncludes for #else
        TEST_CASE(def_handleIncludes_ifelse2);
        TEST_CASE(def_handleIncludes_ifelse3);   // #4868 - crash

        TEST_CASE(def_valueWithParentheses); // #3531

        // Using -U to undefine symbols
        TEST_CASE(undef1);
        TEST_CASE(undef2);
        TEST_CASE(undef3);
        TEST_CASE(undef4);
        TEST_CASE(undef5);
        TEST_CASE(undef6);
        TEST_CASE(undef7);
        TEST_CASE(undef8);
        TEST_CASE(undef9);
        TEST_CASE(undef10);

        TEST_CASE(handleUndef);

        TEST_CASE(macroChar);

        TEST_CASE(validateCfg);

        TEST_CASE(if_sizeof);

        TEST_CASE(double_include); // #5717
        TEST_CASE(invalid_ifs)// #5909
    }


    void readCode1() {
        const char code[] = " \t a //\n"
                            "  #aa\t /* remove this */\tb  \r\n";
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        std::istringstream istr(code);
        std::string codestr(preprocessor.read(istr,"test.c"));
        ASSERT_EQUALS("a\n#aa b\n", codestr);
    }

    void readCode2() {
        const char code[] = "R\"( \" /* abc */ \n)\";";
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        std::istringstream istr(code);
        std::string codestr(preprocessor.read(istr,"test.c"));
        ASSERT_EQUALS("\" \\\" /* abc */ \\n\"\n;", codestr);
    }

    void readCode3() {
        const char code[] = "func(#errorname)";
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        std::istringstream istr(code);
        std::string codestr(preprocessor.read(istr,"test.c"));
        ASSERT_EQUALS("func(#errorname)", codestr);
    }

    void readCode4() {
        const char code[] = "char c = '\\ ';";
        Settings settings;
        errout.str("");
        Preprocessor preprocessor(&settings, this);
        std::istringstream istr(code);
        ASSERT_EQUALS("char c = '\\ ';", preprocessor.read(istr,"test.c"));
        ASSERT_EQUALS("", errout.str());
    }


    void utf16() {
        Settings settings;
        Preprocessor preprocessor(&settings, this);

        // a => a
        {
            const char code[] = { (char)0xff, (char)0xfe, 'a', '\0' };
            std::string s(code, sizeof(code));
            std::istringstream istr(s);
            ASSERT_EQUALS("a", preprocessor.read(istr, "test.c"));
        }

        {
            const char code[] = { (char)0xfe, (char)0xff, '\0', 'a' };
            std::string s(code, sizeof(code));
            std::istringstream istr(s);
            ASSERT_EQUALS("a", preprocessor.read(istr, "test.c"));
        }

        // extended char => 0xff
        {
            const char code[] = { (char)0xff, (char)0xfe, 'a', 'a' };
            std::string s(code, sizeof(code));
            std::istringstream istr(s);
            const char expected[] = { (char)0xff, 0 };
            ASSERT_EQUALS(expected, preprocessor.read(istr, "test.c"));
        }

        {
            const char code[] = { (char)0xfe, (char)0xff, 'a', 'a' };
            std::string s(code, sizeof(code));
            std::istringstream istr(s);
            const char expected[] = { (char)0xff, 0 };
            ASSERT_EQUALS(expected, preprocessor.read(istr, "test.c"));
        }

        // \r\n => \n
        {
            const char code[] = { (char)0xff, (char)0xfe, '\r', '\0', '\n', '\0' };
            std::string s(code, sizeof(code));
            std::istringstream istr(s);
            ASSERT_EQUALS("\n", preprocessor.read(istr, "test.c"));
        }

        {
            const char code[] = { (char)0xfe, (char)0xff, '\0', '\r', '\0', '\n' };
            std::string s(code, sizeof(code));
            std::istringstream istr(s);
            ASSERT_EQUALS("\n", preprocessor.read(istr, "test.c"));
        }
    }


    void removeComments() {
        Settings settings;
        Preprocessor preprocessor(&settings, this);

        // #3837 - asm comments
        const char code[] = "void test(void) {\n"
                            "   __asm\n"
                            "   {\n"
                            "      ;---- тест\n"
                            "   }\n"
                            "}\n";
        ASSERT_EQUALS(true, std::string::npos == preprocessor.removeComments(code, "3837.c").find("----"));

        ASSERT_EQUALS(" __asm123", preprocessor.removeComments(" __asm123", "3837.cpp"));
        ASSERT_EQUALS("\" __asm { ; } \"", preprocessor.removeComments("\" __asm { ; } \"", "3837.cpp"));
        ASSERT_EQUALS("__asm__ volatile { \"\" }", preprocessor.removeComments("__asm__ volatile { \"\" }", "3837.cpp"));

        // #4873
        ASSERT_EQUALS("__asm { }", preprocessor.removeComments("__asm { /* This is a comment */ }", "4873.cpp"));

        // #5169
        ASSERT_EQUALS("#define A(B) __asm__(\"int $3\"); int wait=1;\n",
                      preprocessor.removeComments("#define A(B) __asm__(\"int $3\"); /**/ int wait=1;\n", "5169.c"));
    }


    void Bug2190219() {
        const char filedata[] = "int main()\n"
                                "{\n"
                                "#ifdef __cplusplus\n"
                                "    int* flags = new int[10];\n"
                                "#else\n"
                                "    int* flags = (int*)malloc((10)*sizeof(int));\n"
                                "#endif\n"
                                "\n"
                                "#ifdef __cplusplus\n"
                                "    delete [] flags;\n"
                                "#else\n"
                                "    free(flags);\n"
                                "#endif\n"
                                "}\n";

        // Expected result..
        std::map<std::string, std::string> expected;
        expected[""]          = "int main()\n"
                                "{\n"
                                "\n"
                                "\n"
                                "\n"
                                "int* flags = (int*)malloc((10)*sizeof(int));\n"
                                "\n"
                                "\n"
                                "\n"
                                "\n"
                                "\n"
                                "free(flags);\n"
                                "\n"
                                "}\n";

        expected["__cplusplus"] = "int main()\n"
                                  "{\n"
                                  "\n"
                                  "int* flags = new int[10];\n"
                                  "\n"
                                  "\n"
                                  "\n"
                                  "\n"
                                  "\n"
                                  "delete [] flags;\n"
                                  "\n"
                                  "\n"
                                  "\n"
                                  "}\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS(expected[""], actual[""]);
        ASSERT_EQUALS(expected["__cplusplus"], actual["__cplusplus"]);
        ASSERT_EQUALS(2, static_cast<unsigned int>(actual.size()));
    }


    void test1() {
        const char filedata[] = "#ifdef  WIN32 \n"
                                "    abcdef\n"
                                "#else  \n"
                                "    qwerty\n"
                                "#endif  \n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS("\n\n\nqwerty\n\n", actual[""]);
        ASSERT_EQUALS("\nabcdef\n\n\n\n", actual["WIN32"]);
        ASSERT_EQUALS(2, static_cast<unsigned int>(actual.size()));
    }

    void test2() {
        const char filedata[] = "# ifndef WIN32\n"
                                "    \" # ifdef WIN32\" // a comment\n"
                                "   #   else  \n"
                                "    qwerty\n"
                                "  # endif  \n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS("\n\" # ifdef WIN32\"\n\n\n\n", actual[""]);
        ASSERT_EQUALS("\n\n\nqwerty\n\n", actual["WIN32"]);
        ASSERT_EQUALS(2, static_cast<unsigned int>(actual.size()));
    }

    void test3() {
        const char filedata[] = "#ifdef ABC\n"
                                "a\n"
                                "#ifdef DEF\n"
                                "b\n"
                                "#endif\n"
                                "c\n"
                                "#endif\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS("\n\n\n\n\n\n\n", actual[""]);
        ASSERT_EQUALS("\na\n\n\n\nc\n\n", actual["ABC"]);
        ASSERT_EQUALS("\na\n\nb\n\nc\n\n", actual["ABC;DEF"]);
        ASSERT_EQUALS(3, static_cast<unsigned int>(actual.size()));
    }

    void test4() {
        const char filedata[] = "#ifdef ABC\n"
                                "A\n"
                                "#endif\t\n"
                                "#ifdef ABC\n"
                                "A\n"
                                "#endif\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS("\n\n\n\n\n\n", actual[""]);
        ASSERT_EQUALS("\nA\n\n\nA\n\n", actual["ABC"]);
        ASSERT_EQUALS(2, static_cast<unsigned int>(actual.size()));
    }

    void test5() {
        const char filedata[] = "#ifdef ABC\n"
                                "A\n"
                                "#else\n"
                                "B\n"
                                "#ifdef DEF\n"
                                "C\n"
                                "#endif\n"
                                "#endif\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS("\n\n\nB\n\n\n\n\n", actual[""]);
        ASSERT_EQUALS("\nA\n\n\n\n\n\n\n", actual["ABC"]);
        ASSERT_EQUALS("\n\n\nB\n\nC\n\n\n", actual["DEF"]);
        ASSERT_EQUALS(3, static_cast<unsigned int>(actual.size()));
    }

    void test6() {
        const char filedata[] = "#if(A)\n"
                                "#if ( A ) \n"
                                "#if A\n"
                                "#if defined((A))\n"
                                "#elif defined (A)\n";

        std::istringstream istr(filedata);
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        const std::string actual(preprocessor.read(istr, "test.c"));

        // Compare results..
        ASSERT_EQUALS("#if A\n#if A\n#if A\n#if defined(A)\n#elif defined(A)\n", actual);
    }

    void test7() {
        const char filedata[] = "#ifdef ABC\n"
                                "A\n"
                                "#ifdef ABC\n"
                                "B\n"
                                "#endif\n"
                                "#endif\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        errout.str("");
        preprocessor.preprocess(istr, actual, "file.c");

        // Make sure an error message is written..
        TODO_ASSERT_EQUALS("[file.c:3]: (error) ABC is already guaranteed to be defined\n",
                           "",
                           errout.str());

        // Compare results..
        ASSERT_EQUALS("\n\n\n\n\n\n", actual[""]);
        ASSERT_EQUALS("\nA\n\nB\n\n\n", actual["ABC"]);
        ASSERT_EQUALS(2, static_cast<unsigned int>(actual.size()));

        test7a();
        test7b();
        test7c();
        test7d();
    }

    void test7a() {
        const char filedata[] = "#ifndef ABC\n"
                                "A\n"
                                "#ifndef ABC\n"
                                "B\n"
                                "#endif\n"
                                "#endif\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        errout.str("");
        preprocessor.preprocess(istr, actual, "file.c");

        // Make sure an error message is written..
        TODO_ASSERT_EQUALS("[file.c:3]: (error) ABC is already guaranteed NOT to be defined\n",
                           "", errout.str());

        // Compare results..
        ASSERT_EQUALS(2, static_cast<unsigned int>(actual.size()));
    }

    void test7b() {
        const char filedata[] = "#ifndef ABC\n"
                                "A\n"
                                "#ifdef ABC\n"
                                "B\n"
                                "#endif\n"
                                "#endif\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        errout.str("");
        preprocessor.preprocess(istr, actual, "file.c");

        // Make sure an error message is written..
        TODO_ASSERT_EQUALS("[file.c:3]: (error) ABC is already guaranteed NOT to be defined\n",
                           "", errout.str());

        // Compare results..
        ASSERT_EQUALS(2, static_cast<unsigned int>(actual.size()));
    }

    void test7c() {
        const char filedata[] = "#ifdef ABC\n"
                                "A\n"
                                "#ifndef ABC\n"
                                "B\n"
                                "#endif\n"
                                "#endif\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        errout.str("");
        preprocessor.preprocess(istr, actual, "file.c");

        // Make sure an error message is written..
        TODO_ASSERT_EQUALS("[file.c:3]: (error) ABC is already guaranteed to be defined\n",
                           "",
                           errout.str());

        // Compare results..
        ASSERT_EQUALS(2, static_cast<unsigned int>(actual.size()));
    }

    void test7d() {
        const char filedata[] = "#if defined(ABC)\n"
                                "A\n"
                                "#if defined(ABC)\n"
                                "B\n"
                                "#endif\n"
                                "#endif\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        errout.str("");
        preprocessor.preprocess(istr, actual, "file.c");

        // Make sure an error message is written..
        TODO_ASSERT_EQUALS("[file.c:3]: (error) ABC is already guaranteed to be defined\n",
                           "",
                           errout.str());

        // Compare results..
        ASSERT_EQUALS(2, static_cast<unsigned int>(actual.size()));
    }

    void test7e() {
        const char filedata[] = "#ifdef ABC\n"
                                "#file \"test.h\"\n"
                                "#ifndef test_h\n"
                                "#define test_h\n"
                                "#ifdef ABC\n"
                                "#endif\n"
                                "#endif\n"
                                "#endfile\n"
                                "#endif\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        errout.str("");
        preprocessor.preprocess(istr, actual, "file.c");

        // Make sure an error message is written..
        ASSERT_EQUALS("",
                      errout.str());

        // Compare results..
        ASSERT_EQUALS(2, static_cast<unsigned int>(actual.size()));
    }

    void test8() {
        const char filedata[] = "#if A == 1\n"
                                "1\n"
                                "#endif\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        errout.str("");
        preprocessor.preprocess(istr, actual, "file.c");

        // No error..
        ASSERT_EQUALS("", errout.str());

        // Compare results..
        ASSERT_EQUALS(2U, actual.size());
        ASSERT_EQUALS("\n\n\n", actual[""]);
        ASSERT_EQUALS("\n1\n\n", actual["A=1"]);
    }

    void test9() {
        const char filedata[] = "#if\n"
                                "#else\n"
                                "#endif\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        settings._maxConfigs = 1;
        settings.userDefines = "X";
        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c"); // <- don't crash
    }

    void test10() { // Ticket #5139
        const char filedata[] = "#define foo a.foo\n"
                                "#define bar foo\n"
                                "#define baz bar+0\n"
                                "#if 0\n"
                                "#endif";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");
    }

    void error1() {
        const char filedata[] = "#ifdef A\n"
                                ";\n"
                                "#else\n"
                                "#error abcd\n"
                                "#endif\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        errout.str("");
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS(2, static_cast<unsigned int>(actual.size()));
        ASSERT_EQUALS("", actual[""]);
        ASSERT_EQUALS("\n;\n\n\n\n", actual["A"]);

    }


    void error2() {
        errout.str("");

        const char filedata[] = "#error \xAB\n"
                                "#warning \xAB\n"
                                "123";

        // Read string..
        std::istringstream istr(filedata);
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        ASSERT_EQUALS("#error\n\n123", preprocessor.read(istr,"test.c"));
    }


    void error3() {
        errout.str("");
        Settings settings;
        settings.userDefines = "__cplusplus";
        Preprocessor preprocessor(&settings, this);
        const std::string code("#error hello world!\n");
        preprocessor.getcode(code, "X", "test.c");
        ASSERT_EQUALS("[test.c:1]: (error) #error hello world!\n", errout.str());
    }

    // Ticket #2919 - wrong filename reported for #error
    void error4() {
        // In included file
        {
            errout.str("");
            Settings settings;
            settings.userDefines = "TEST";
            Preprocessor preprocessor(&settings, this);
            const std::string code("#file \"ab.h\"\n#error hello world!\n#endfile");
            preprocessor.getcode(code, "TEST", "test.c");
            ASSERT_EQUALS("[ab.h:1]: (error) #error hello world!\n", errout.str());
        }

        // After including a file
        {
            errout.str("");
            Settings settings;
            settings.userDefines = "TEST";
            Preprocessor preprocessor(&settings, this);
            const std::string code("#file \"ab.h\"\n\n#endfile\n#error aaa");
            preprocessor.getcode(code, "TEST", "test.c");
            ASSERT_EQUALS("[test.c:2]: (error) #error aaa\n", errout.str());
        }
    }

    void error5() {
        errout.str("");
        Settings settings;
        settings.userDefines = "FOO";
        settings._force = true; // No message if --force is given
        Preprocessor preprocessor(&settings, this);
        const std::string code("#error hello world!\n");
        preprocessor.getcode(code, "X", "test.c");
        ASSERT_EQUALS("", errout.str());
    }

    void if0_exclude() {
        Settings settings;
        Preprocessor preprocessor(&settings, this);

        std::istringstream code("#if 0\n"
                                "A\n"
                                "#endif\n"
                                "B\n");
        ASSERT_EQUALS("#if 0\n\n#endif\nB\n", preprocessor.read(code,""));

        std::istringstream code2("#if (0)\n"
                                 "A\n"
                                 "#endif\n"
                                 "B\n");
        ASSERT_EQUALS("#if 0\n\n#endif\nB\n", preprocessor.read(code2,""));
    }

    void if0_whitespace() {
        Settings settings;
        Preprocessor preprocessor(&settings, this);

        std::istringstream code(" # if  0 \n"
                                "A\n"
                                " # endif \n"
                                "B\n");
        ASSERT_EQUALS("#if 0\n\n#endif\nB\n", preprocessor.read(code,""));
    }

    void if0_else() {
        Settings settings;
        Preprocessor preprocessor(&settings, this);

        std::istringstream code("#if 0\n"
                                "A\n"
                                "#else\n"
                                "B\n"
                                "#endif\n"
                                "C\n");
        ASSERT_EQUALS("#if 0\n\n#else\nB\n#endif\nC\n", preprocessor.read(code,""));

        std::istringstream code2("#if 1\n"
                                 "A\n"
                                 "#else\n"
                                 "B\n"
                                 "#endif\n"
                                 "C\n");
        TODO_ASSERT_EQUALS("#if 1\nA\n#else\n\n#endif\nC\n",
                           "#if 1\nA\n#else\nB\n#endif\nC\n", preprocessor.read(code2,""));
    }

    void if0_elif() {
        Settings settings;
        Preprocessor preprocessor(&settings, this);

        std::istringstream code("#if 0\n"
                                "A\n"
                                "#elif 1\n"
                                "B\n"
                                "#endif\n"
                                "C\n");
        ASSERT_EQUALS("#if 0\n\n#elif 1\nB\n#endif\nC\n", preprocessor.read(code,""));
    }

    void if0_include_1() {
        Settings settings;
        Preprocessor preprocessor(&settings, this);

        std::istringstream code("#if 0\n"
                                "#include \"a.h\"\n"
                                "#endif\n"
                                "AB\n");
        ASSERT_EQUALS("#if 0\n\n#endif\nAB\n", preprocessor.read(code,""));
    }

    void if0_include_2() {
        Settings settings;
        Preprocessor preprocessor(&settings, this);

        std::istringstream code("#if 0\n"
                                "#include \"a.h\"\n"
                                "#ifdef WIN32\n"
                                "#else\n"
                                "#endif\n"
                                "#endif\n"
                                "AB\n");
        ASSERT_EQUALS("#if 0\n\n#ifdef WIN32\n#else\n#endif\n#endif\nAB\n", preprocessor.read(code,""));
    }

    void includeguard1() {
        // Handling include guards..
        const char filedata[] = "#file \"abc.h\"\n"
                                "#ifndef abcH\n"
                                "#define abcH\n"
                                "#endif\n"
                                "#endfile\n"
                                "#ifdef ABC\n"
                                "#endif";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        // Expected configurations: "" and "ABC"
        ASSERT_EQUALS(2, static_cast<unsigned int>(actual.size()));
    }

    void includeguard2() {
        // Handling include guards..
        const char filedata[] = "#file \"abc.h\"\n"
                                "foo\n"
                                "#ifdef ABC\n"
                                "\n"
                                "#endif\n"
                                "#endfile\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        // Expected configurations: "" and "ABC"
        ASSERT_EQUALS(2, static_cast<unsigned int>(actual.size()));
        ASSERT_EQUALS(true, actual.find("") != actual.end());
        ASSERT_EQUALS(true, actual.find("ABC") != actual.end());
    }


    void ifdefwithfile() {
        // Handling include guards..
        const char filedata[] = "#ifdef ABC\n"
                                "#file \"abc.h\"\n"
                                "class A{};/*\n\n\n\n\n\n\n*/\n"
                                "#endfile\n"
                                "#endif\n"
                                "int main() {}\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        // Expected configurations: "" and "ABC"
        ASSERT_EQUALS(2, static_cast<unsigned int>(actual.size()));
        ASSERT_EQUALS("\n#file \"abc.h\"\n\n\n\n\n\n\n\n\n#endfile\n\nint main() {}\n", actual[""]);
        ASSERT_EQUALS("\n#file \"abc.h\"\nclass A{};\n\n\n\n\n\n\n\n#endfile\n\nint main() {}\n", actual["ABC"]);
    }

    void newlines() {
        const char filedata[] = "\r\r\n\n";

        // Preprocess
        std::istringstream istr(filedata);
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        ASSERT_EQUALS("\n\n\n", preprocessor.read(istr, "test.c"));
    }



    void comments1() {
        {
            const char filedata[] = "/*\n"
                                    "#ifdef WIN32\n"
                                    "#endif\n"
                                    "*/\n";

            // Preprocess => actual result..
            std::istringstream istr(filedata);
            std::map<std::string, std::string> actual;
            Settings settings;
            Preprocessor preprocessor(&settings, this);
            preprocessor.preprocess(istr, actual, "file.c");

            // Compare results..
            ASSERT_EQUALS("\n\n\n\n", actual[""]);
            ASSERT_EQUALS(1, static_cast<unsigned int>(actual.size()));
        }

        {
            const char filedata[] = "/*\n"
                                    "\x080 #ifdef WIN32\n"
                                    "#endif\n"
                                    "*/\n";

            // Preprocess => actual result..
            std::istringstream istr(filedata);
            std::map<std::string, std::string> actual;
            Settings settings;
            Preprocessor preprocessor(&settings, this);
            preprocessor.preprocess(istr, actual, "file.c");

            // Compare results..
            ASSERT_EQUALS("\n\n\n\n", actual[""]);
            ASSERT_EQUALS(1, static_cast<unsigned int>(actual.size()));
        }

        {
            const char filedata[] = "void f()\n"
                                    "{\n"
                                    "  *p = a / *b / *c;\n"
                                    "}\n";

            // Preprocess => actual result..
            std::istringstream istr(filedata);
            std::map<std::string, std::string> actual;
            Settings settings;
            Preprocessor preprocessor(&settings, this);
            preprocessor.preprocess(istr, actual, "file.c");

            // Compare results..
            ASSERT_EQUALS("void f()\n{\n*p = a / *b / *c;\n}\n", actual[""]);
            ASSERT_EQUALS(1, static_cast<unsigned int>(actual.size()));
        }
    }



    void if0() {
        const char filedata[] = " # if /* comment */  0 // comment\n"
                                "#ifdef WIN32\n"
                                "#endif\n"
                                "#endif\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS("\n\n\n\n", actual[""]);
        ASSERT_EQUALS(1, static_cast<unsigned int>(actual.size()));
    }

    void if1() {
        const char filedata[] = " # if /* comment */  1 // comment\n"
                                "ABC\n"
                                " # endif \n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS("\nABC\n\n", actual[""]);
        ASSERT_EQUALS(1, static_cast<unsigned int>(actual.size()));
    }


    void elif() {
        {
            const char filedata[] = "#if DEF1\n"
                                    "ABC\n"
                                    "#elif DEF2\n"
                                    "DEF\n"
                                    "#endif\n";

            // Preprocess => actual result..
            std::istringstream istr(filedata);
            std::map<std::string, std::string> actual;
            Settings settings;
            Preprocessor preprocessor(&settings, this);
            preprocessor.preprocess(istr, actual, "file.c");

            // Compare results..
            ASSERT_EQUALS("\n\n\n\n\n", actual[""]);
            ASSERT_EQUALS("\nABC\n\n\n\n", actual["DEF1"]);
            ASSERT_EQUALS("\n\n\nDEF\n\n", actual["DEF2"]);
            ASSERT_EQUALS(3, static_cast<unsigned int>(actual.size()));
        }

        {
            const char filedata[] = "#if(defined DEF1)\n"
                                    "ABC\n"
                                    "#elif(defined DEF2)\n"
                                    "DEF\n"
                                    "#else\n"
                                    "GHI\n"
                                    "#endif\n";

            // Preprocess => actual result..
            std::istringstream istr(filedata);
            std::map<std::string, std::string> actual;
            Settings settings;
            Preprocessor preprocessor(&settings, this);
            preprocessor.preprocess(istr, actual, "file.c");

            // Compare results..
            ASSERT_EQUALS("\n\n\n\n\nGHI\n\n", actual[""]);
            ASSERT_EQUALS("\nABC\n\n\n\n\n\n", actual["DEF1"]);
            ASSERT_EQUALS("\n\n\nDEF\n\n\n\n", actual["DEF2"]);
            ASSERT_EQUALS(3, static_cast<unsigned int>(actual.size()));
        }
    }



    void match_cfg_def() {
        Preprocessor preprocessor(nullptr, this);

        {
            std::map<std::string, std::string> cfg;
            ASSERT_EQUALS(false, preprocessor.match_cfg_def(cfg, "A>1||defined(B)"));
        }

        {
            std::map<std::string, std::string> cfg;
            cfg["A"] = "";
            cfg["B"] = "";
            ASSERT_EQUALS(true, preprocessor.match_cfg_def(cfg, "defined(A)&&defined(B)"));
        }

        {
            std::map<std::string, std::string> cfg;
            cfg["ABC"] = "";

            ASSERT_EQUALS(false, preprocessor.match_cfg_def(cfg, "defined(A)"));
            ASSERT_EQUALS(true, preprocessor.match_cfg_def(cfg, "!defined(A)"));

            ASSERT_EQUALS(false, preprocessor.match_cfg_def(cfg, "!defined(ABC)&&!defined(DEF)"));
            ASSERT_EQUALS(true, preprocessor.match_cfg_def(cfg, "!defined(A)&&!defined(B)"));
        }

        {
            std::map<std::string, std::string> cfg;
            cfg["A"] = "1";
            cfg["B"] = "2";

            ASSERT_EQUALS(true, preprocessor.match_cfg_def(cfg, "A==1"));
            ASSERT_EQUALS(true, preprocessor.match_cfg_def(cfg, "A<2"));
            ASSERT_EQUALS(false, preprocessor.match_cfg_def(cfg, "A==2"));
            ASSERT_EQUALS(false, preprocessor.match_cfg_def(cfg, "A<1"));
            ASSERT_EQUALS(false, preprocessor.match_cfg_def(cfg, "A>=1&&B<=A"));
            ASSERT_EQUALS(true, preprocessor.match_cfg_def(cfg, "A==1 && A==1"));
        }
    }


    void if_cond1() {
        const char filedata[] = "#if LIBVER>100\n"
                                "    A\n"
                                "#else\n"
                                "    B\n"
                                "#endif\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS(1, static_cast<unsigned int>(actual.size()));
        ASSERT_EQUALS("\n\n\nB\n\n", actual[""]);
        TODO_ASSERT_EQUALS("\nA\n\n\n\n",
                           "", actual["LIBVER=101"]);
    }

    void if_cond2() {
        const char filedata[] = "#ifdef A\n"
                                "a\n"
                                "#endif\n"
                                "#if defined(A) && defined(B)\n"
                                "ab\n"
                                "#endif\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS(3, static_cast<unsigned int>(actual.size()));
        ASSERT_EQUALS("\n\n\n\n\n\n", actual[""]);
        ASSERT_EQUALS("\na\n\n\n\n\n", actual["A"]);
        ASSERT_EQUALS("\na\n\n\nab\n\n", actual["A;B"]);
        if_cond2b();
        if_cond2c();
        if_cond2d();
        if_cond2e();
    }

    void if_cond2b() {
        const char filedata[] = "#ifndef A\n"
                                "!a\n"
                                "#ifdef B\n"
                                "b\n"
                                "#endif\n"
                                "#else\n"
                                "a\n"
                                "#endif\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS(3, static_cast<unsigned int>(actual.size()));
        ASSERT_EQUALS("\n!a\n\n\n\n\n\n\n", actual[""]);
        ASSERT_EQUALS("\n\n\n\n\n\na\n\n", actual["A"]);
        ASSERT_EQUALS("\n!a\n\nb\n\n\n\n\n", actual["B"]);
    }

    void if_cond2c() {
        const char filedata[] = "#ifndef A\n"
                                "!a\n"
                                "#ifdef B\n"
                                "b\n"
                                "#else\n"
                                "!b\n"
                                "#endif\n"
                                "#else\n"
                                "a\n"
                                "#endif\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS(3, static_cast<unsigned int>(actual.size()));
        ASSERT_EQUALS("\n!a\n\n\n\n!b\n\n\n\n\n", actual[""]);
        ASSERT_EQUALS("\n\n\n\n\n\n\n\na\n\n", actual["A"]);
        ASSERT_EQUALS("\n!a\n\nb\n\n\n\n\n\n\n", actual["B"]);
    }

    void if_cond2d() {
        const char filedata[] = "#ifndef A\n"
                                "!a\n"
                                "#ifdef B\n"
                                "b\n"
                                "#else\n"
                                "!b\n"
                                "#endif\n"
                                "#else\n"
                                "a\n"
                                "#ifdef B\n"
                                "b\n"
                                "#else\n"
                                "!b\n"
                                "#endif\n"
                                "#endif\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS(4, static_cast<unsigned int>(actual.size()));
        ASSERT_EQUALS("\n!a\n\n\n\n!b\n\n\n\n\n\n\n\n\n\n", actual[""]);
        ASSERT_EQUALS("\n\n\n\n\n\n\n\na\n\n\n\n!b\n\n\n", actual["A"]);
        ASSERT_EQUALS("\n\n\n\n\n\n\n\na\n\nb\n\n\n\n\n", actual["A;B"]);
        ASSERT_EQUALS("\n!a\n\nb\n\n\n\n\n\n\n\n\n\n\n\n", actual["B"]);
    }

    void if_cond2e() {
        const char filedata[] = "#if !defined(A)\n"
                                "!a\n"
                                "#elif !defined(B)\n"
                                "!b\n"
                                "#endif\n";

        // Preprocess => actual result..
        errout.str("");
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        settings.debug = settings.debugwarnings = true;
        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS(3, static_cast<unsigned int>(actual.size()));
        ASSERT_EQUALS("\n!a\n\n\n\n", actual[""]);
        ASSERT_EQUALS("\n\n\n!b\n\n", actual["A"]);
        TODO_ASSERT_EQUALS("\n\n\n\n\n", "", actual["A;B"]);
        ASSERT_EQUALS("", errout.str());
    }

    void if_cond3() {
        const char filedata[] = "#ifdef A\n"
                                "a\n"
                                "#if defined(B) && defined(C)\n"
                                "abc\n"
                                "#endif\n"
                                "#endif\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS(3, static_cast<unsigned int>(actual.size()));
        ASSERT_EQUALS("\n\n\n\n\n\n", actual[""]);
        ASSERT_EQUALS("\na\n\n\n\n\n", actual["A"]);
        ASSERT_EQUALS("\na\n\nabc\n\n\n", actual["A;B;C"]);
    }

    void if_cond4() {
        {
            const char filedata[] = "#define A\n"
                                    "#define B\n"
                                    "#if defined A || defined B\n"
                                    "ab\n"
                                    "#endif\n";

            // Preprocess => actual result..
            std::istringstream istr(filedata);
            std::map<std::string, std::string> actual;
            Settings settings;
            Preprocessor preprocessor(&settings, this);
            preprocessor.preprocess(istr, actual, "file.c");

            // Compare results..
            ASSERT_EQUALS(1, static_cast<unsigned int>(actual.size()));
            ASSERT_EQUALS("\n\n\nab\n\n", actual[""]);
        }

        {
            const char filedata[] = "#if A\n"
                                    "{\n"
                                    "#if (defined(B))\n"
                                    "foo();\n"
                                    "#endif\n"
                                    "}\n"
                                    "#endif\n";

            // Preprocess => actual result..
            std::istringstream istr(filedata);
            std::map<std::string, std::string> actual;
            Settings settings;
            Preprocessor preprocessor(&settings, this);
            preprocessor.preprocess(istr, actual, "file.c");

            // Compare results..
            ASSERT_EQUALS(3, static_cast<unsigned int>(actual.size()));
            ASSERT_EQUALS("\n\n\n\n\n\n\n", actual[""]);
            ASSERT_EQUALS("\n{\n\n\n\n}\n\n", actual["A"]);
            ASSERT_EQUALS("\n{\n\nfoo();\n\n}\n\n", actual["A;B"]);
        }

        {
            const char filedata[] = "#define A\n"
                                    "#define B\n"
                                    "#if (defined A) || defined (B)\n"
                                    "ab\n"
                                    "#endif\n";

            // Preprocess => actual result..
            std::istringstream istr(filedata);
            std::map<std::string, std::string> actual;
            Settings settings;
            Preprocessor preprocessor(&settings, this);
            preprocessor.preprocess(istr, actual, "file.c");

            // Compare results..
            ASSERT_EQUALS(1, static_cast<unsigned int>(actual.size()));
            ASSERT_EQUALS("\n\n\nab\n\n", actual[""]);
        }

        {
            const char filedata[] = "#if (A)\n"
                                    "foo();\n"
                                    "#endif\n";

            // Preprocess => actual result..
            std::istringstream istr(filedata);
            std::map<std::string, std::string> actual;
            Settings settings;
            Preprocessor preprocessor(&settings, this);
            preprocessor.preprocess(istr, actual, "file.c");

            // Compare results..
            ASSERT_EQUALS(2, static_cast<unsigned int>(actual.size()));
            ASSERT_EQUALS("\n\n\n", actual[""]);
            ASSERT_EQUALS("\nfoo();\n\n", actual["A"]);
        }

        {
            const char filedata[] = "#if! A\n"
                                    "foo();\n"
                                    "#endif\n";

            // Preprocess => actual result..
            std::istringstream istr(filedata);
            std::map<std::string, std::string> actual;
            Settings settings;
            Preprocessor preprocessor(&settings, this);
            preprocessor.preprocess(istr, actual, "file.c");

            // Compare results..
            TODO_ASSERT_EQUALS(2, 1, static_cast<unsigned int>(actual.size()));
            ASSERT_EQUALS("\nfoo();\n\n", actual[""]);
        }
    }

    void if_cond5() {
        const char filedata[] = "#if defined(A) && defined(B)\n"
                                "ab\n"
                                "#endif\n"
                                "cd\n"
                                "#if defined(B) && defined(A)\n"
                                "ef\n"
                                "#endif\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS(2, static_cast<unsigned int>(actual.size()));
        ASSERT_EQUALS("\n\n\ncd\n\n\n\n", actual[""]);
        ASSERT_EQUALS("\nab\n\ncd\n\nef\n\n", actual["A;B"]);
    }

    void if_cond6() {
        const char filedata[] = "\n"
                                "#if defined(A) && defined(B))\n"
                                "#endif\n";

        errout.str("");

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS("[file.c:2]: (error) mismatching number of '(' and ')' in this line: defined(A)&&defined(B))\n", errout.str());
    }

    void if_cond8() {
        const char filedata[] = "#if defined(A) + defined(B) + defined(C) != 1\n"
                                "#endif\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS(1, (int)actual.size());
        ASSERT_EQUALS("\n\n", actual[""]);
    }


    void if_cond9() {
        const char filedata[] = "#if !defined _A\n"
                                "abc\n"
                                "#endif\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS(1, (int)actual.size());
        ASSERT_EQUALS("\nabc\n\n", actual[""]);
    }

    void if_cond10() {
        const char filedata[] = "#if !defined(a) && !defined(b)\n"
                                "#if defined(and)\n"
                                "#endif\n"
                                "#endif\n";

        // Preprocess => don't crash..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");
    }

    void if_cond11() {
        errout.str("");
        const char filedata[] = "#if defined(L_fixunssfdi) && LIBGCC2_HAS_SF_MODE\n"
                                "#if LIBGCC2_HAS_DF_MODE\n"
                                "#elif FLT_MANT_DIG < W_TYPE_SIZE\n"
                                "#endif\n"
                                "#endif\n";
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");
        ASSERT_EQUALS("", errout.str());
    }

    void if_cond12() {
        const char filedata[] = "#define A (1)\n"
                                "#if A == 1\n"
                                ";\n"
                                "#endif\n";
        Preprocessor preprocessor(nullptr, this);
        ASSERT_EQUALS("\n\n;\n\n", preprocessor.getcode(filedata,"",""));
    }

    void if_cond13() {
        const char filedata[] = "#if ('A' == 0x41)\n"
                                "123\n"
                                "#endif\n";
        Preprocessor preprocessor(nullptr, this);
        ASSERT_EQUALS("\n123\n\n", preprocessor.getcode(filedata,"",""));
    }

    void if_cond14() {
        const char filedata[] = "#if !(A)\n"
                                "123\n"
                                "#endif\n";
        Preprocessor preprocessor(nullptr, this);
        ASSERT_EQUALS("\n123\n\n", preprocessor.getcode(filedata,"",""));
    }

    void if_cond15() { // #4456 - segmentation fault
        const char filedata[] = "#if ((A >= B) && (C != D))\n"
                                "#if (E < F(1))\n"
                                "#endif\n"
                                "#endif\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "4456.c");  // <- don't crash in Preprocessor::getcfgs -> Tokenize -> number of template parameters
    }



    void if_or_1() {
        const char filedata[] = "#if defined(DEF_10) || defined(DEF_11)\n"
                                "a1;\n"
                                "#endif\n";

        errout.str("");
        output.str("");

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        settings.debug = settings.debugwarnings = true;
        settings.addEnabled("missingInclude");
        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS(1, (int)actual.size());
        ASSERT_EQUALS("\n\n\n", actual[""]);

        // the "defined(DEF_10) || defined(DEF_11)" are not handled correctly..
        ASSERT_EQUALS("(debug) unhandled configuration: defined(DEF_10)||defined(DEF_11)\n", errout.str());
        TODO_ASSERT_EQUALS(2, 1, actual.size());
        TODO_ASSERT_EQUALS("\na1;\n\n",
                           "", actual["DEF_10"]);

    }

    void if_or_2() {
        const std::string code("#if X || Y\n"
                               "a1;\n"
                               "#endif\n");
        Preprocessor preprocessor(nullptr, this);
        ASSERT_EQUALS("\na1;\n\n", preprocessor.getcode(code, "X", "test.c"));
        ASSERT_EQUALS("\na1;\n\n", preprocessor.getcode(code, "Y", "test.c"));
    }

    void if_macro_eq_macro() {
        const std::string code("#define A B\n"
                               "#define B 1\n"
                               "#define C 1\n"
                               "#if A == C\n"
                               "Wilma\n"
                               "#else\n"
                               "Betty\n"
                               "#endif\n");
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        std::istringstream istr(code);
        std::map<std::string, std::string> actual;
        preprocessor.preprocess(istr, actual, "file.c");

        ASSERT_EQUALS("\n\n\n\nWilma\n\n\n\n", actual[""]);
    }

    void ticket_3675() {
        const std::string code("#ifdef YYSTACKSIZE\n"
                               "#define YYMAXDEPTH YYSTACKSIZE\n"
                               "#else\n"
                               "#define YYSTACKSIZE YYMAXDEPTH\n"
                               "#endif\n"
                               "#if YYDEBUG\n"
                               "#endif\n");
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        std::istringstream istr(code);
        std::map<std::string, std::string> actual;
        preprocessor.preprocess(istr, actual, "file.c");

        // There's nothing to assert. It just needs to not hang.
    }

    void ticket_3699() {
        const std::string code("#define INLINE __forceinline\n"
                               "#define inline __forceinline\n"
                               "#define __forceinline inline\n"
                               "#if !defined(_WIN32)\n"
                               "#endif\n"
                               "INLINE inline __forceinline\n"
                              );
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        std::istringstream istr(code);
        std::map<std::string, std::string> actual;
        preprocessor.preprocess(istr, actual, "file.c");

        // First, it must not hang. Second, inline must becomes inline, and __forceinline must become __forceinline.
        ASSERT_EQUALS("\n\n\n\n\n$$$__forceinline $$inline $$__forceinline\n", actual[""]);
    }

    void ticket_4922() { // #4922
        const std::string code("__asm__ \n"
                               "{ int extern __value) 0; (double return (\"\" } extern\n"
                               "__typeof __finite (__finite) __finite __inline \"__GI___finite\");");
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        std::istringstream istr(code);
        std::map<std::string, std::string> actual;
        preprocessor.preprocess(istr, actual, "file.cpp");
    }

    void multiline1() {
        const char filedata[] = "#define str \"abc\"     \\\n"
                                "            \"def\"       \n"
                                "abcdef = str;\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        Preprocessor preprocessor(nullptr, this);
        ASSERT_EQUALS("#define str \"abc\" \"def\"\n\nabcdef = str;\n", preprocessor.read(istr, "test.c"));
    }

    void multiline2() {
        const char filedata[] = "#define sqr(aa) aa * \\\n"
                                "                aa\n"
                                "sqr(5);\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        Preprocessor preprocessor(nullptr, this);
        ASSERT_EQUALS("#define sqr(aa) aa * aa\n\nsqr(5);\n", preprocessor.read(istr, "test.c"));
    }

    void multiline3() {
        const char filedata[] = "const char *str = \"abc\\\n"
                                "def\\\n"
                                "ghi\"\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        Preprocessor preprocessor(nullptr, this);
        ASSERT_EQUALS("const char *str = \"abcdefghi\"\n\n\n", preprocessor.read(istr, "test.c"));
    }

    void multiline4() {
        errout.str("");
        const char filedata[] = "#define A int a = 4;\\ \n"
                                " int b = 5;\n"
                                "A\n";

        // Preprocess => actual result..
        Settings settings;
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS(1, static_cast<unsigned int>(actual.size()));
#ifdef __GNUC__
        ASSERT_EQUALS("\n\n$int $a = $4; $int $b = $5;\n", actual[""]);
#else
        ASSERT_EQUALS("\nint b = 5;\n$int $a = $4;\\\n", actual[""]);
#endif
        ASSERT_EQUALS("", errout.str());
    }

    void multiline5() {
        errout.str("");
        const char filedata[] = "#define ABC int a /*\n"
                                "*/= 4;\n"
                                "int main(){\n"
                                "ABC\n"
                                "}\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS(1, static_cast<unsigned int>(actual.size()));
        ASSERT_EQUALS("\n\nint main(){\n$int $a = $4;\n}\n", actual[""]);
        ASSERT_EQUALS("", errout.str());
    }

    void remove_asm() const {
        std::string str1("#asm\nmov ax,bx\n#endasm");
        Preprocessor::removeAsm(str1);
        ASSERT_EQUALS("asm(\nmov ax,bx\n);", str1);

        std::string str2("\n#asm\nmov ax,bx\n#endasm\n");
        Preprocessor::removeAsm(str2);
        ASSERT_EQUALS("\nasm(\nmov ax,bx\n);\n", str2);
    }

    void if_defined() const {
        {
            const char filedata[] = "#if defined(AAA)\n"
                                    "#endif\n";
            ASSERT_EQUALS("#ifdef AAA\n#endif\n", OurPreprocessor::replaceIfDefined(filedata));
        }

        {
            ASSERT_EQUALS("#elif A\n", OurPreprocessor::replaceIfDefined("#elif defined(A)\n"));
        }
    }

    void if_not_defined() const {
        const char filedata[] = "#if !defined(AAA)\n"
                                "#endif\n";
        ASSERT_EQUALS("#ifndef AAA\n#endif\n", OurPreprocessor::replaceIfDefined(filedata));
    }


    void macro_simple1() const {
        {
            const char filedata[] = "#define AAA(aa) f(aa)\n"
                                    "AAA(5);\n";
            ASSERT_EQUALS("\n$f($5);\n", OurPreprocessor::expandMacros(filedata));
        }

        {
            const char filedata[] = "#define AAA(aa) f(aa)\n"
                                    "AAA (5);\n";
            ASSERT_EQUALS("\n$f($5);\n", OurPreprocessor::expandMacros(filedata));
        }
    }

    void macro_simple2() const {
        const char filedata[] = "#define min(x,y) x<y?x:y\n"
                                "min(a(),b());\n";
        ASSERT_EQUALS("\n$a()<$b()?$a():$b();\n", OurPreprocessor::expandMacros(filedata));
    }

    void macro_simple3() const {
        const char filedata[] = "#define A 4\n"
                                "A AA\n";
        ASSERT_EQUALS("\n$4 AA\n", OurPreprocessor::expandMacros(filedata));
    }

    void macro_simple4() const {
        const char filedata[] = "#define TEMP_1 if( temp > 0 ) return 1;\n"
                                "TEMP_1\n";
        ASSERT_EQUALS("\n$if( $temp > $0 ) $return $1;\n", OurPreprocessor::expandMacros(filedata));
    }

    void macro_simple5() const {
        const char filedata[] = "#define ABC if( temp > 0 ) return 1;\n"
                                "\n"
                                "void foo()\n"
                                "{\n"
                                "    int temp = 0;\n"
                                "    ABC\n"
                                "}\n";
        ASSERT_EQUALS("\n\nvoid foo()\n{\n    int temp = 0;\n    $if( $temp > $0 ) $return $1;\n}\n", OurPreprocessor::expandMacros(filedata));
    }

    void macro_simple6() const {
        const char filedata[] = "#define ABC (a+b+c)\n"
                                "ABC\n";
        ASSERT_EQUALS("\n$($a+$b+$c)\n", OurPreprocessor::expandMacros(filedata));
    }

    void macro_simple7() const {
        const char filedata[] = "#define ABC(str) str\n"
                                "ABC(\"(\")\n";
        ASSERT_EQUALS("\n$\"(\"\n", OurPreprocessor::expandMacros(filedata));
    }

    void macro_simple8() const {
        const char filedata[] = "#define ABC 123\n"
                                "#define ABCD 1234\n"
                                "ABC ABCD\n";
        ASSERT_EQUALS("\n\n$123 $1234\n", OurPreprocessor::expandMacros(filedata));
    }

    void macro_simple9() const {
        const char filedata[] = "#define ABC(a) f(a)\n"
                                "ABC( \"\\\"\" );\n"
                                "ABC( \"g\" );\n";
        ASSERT_EQUALS("\n$f(\"\\\"\");\n$f(\"g\");\n", OurPreprocessor::expandMacros(filedata));
    }

    void macro_simple10() const {
        const char filedata[] = "#define ABC(t) t x\n"
                                "ABC(unsigned long);\n";
        ASSERT_EQUALS("\n$unsigned $long $x;\n", OurPreprocessor::expandMacros(filedata));
    }

    void macro_simple11() const {
        const char filedata[] = "#define ABC(x) delete x\n"
                                "ABC(a);\n";
        ASSERT_EQUALS("\n$delete $a;\n", OurPreprocessor::expandMacros(filedata));
    }

    void macro_simple12() const {
        const char filedata[] = "#define AB ab.AB\n"
                                "AB.CD\n";
        ASSERT_EQUALS("\n$ab.$AB.CD\n", OurPreprocessor::expandMacros(filedata));
    }

    void macro_simple13() const {
        const char filedata[] = "#define TRACE(x)\n"
                                "TRACE(;if(a))\n";
        ASSERT_EQUALS("\n$\n", OurPreprocessor::expandMacros(filedata));
    }

    void macro_simple14() const {
        const char filedata[] = "#define A \"  a  \"\n"
                                "printf(A);\n";
        ASSERT_EQUALS("\nprintf($\"  a  \");\n", OurPreprocessor::expandMacros(filedata));
    }

    void macro_simple15() const {
        const char filedata[] = "#define FOO\"foo\"\n"
                                "FOO\n";
        ASSERT_EQUALS("\n$\"foo\"\n", OurPreprocessor::expandMacros(filedata));
    }

    void macro_simple16() const {  // # 4703
        const char filedata[] = "#define MACRO( A, B, C ) class A##B##C##Creator {};\n"
                                "MACRO( B\t, U , G )";
        ASSERT_EQUALS("\n$class $BUGCreator{};", OurPreprocessor::expandMacros(filedata));
    }

    void macro_simple17() const {  // # 5074 - the Token::isExpandedMacro() doesn't always indicate properly if token comes from macro
        // It would probably be OK if the generated code was
        // "\n123+$123" since the first 123 comes from the source code
        const char filedata[] = "#define MACRO(A) A+123\n"
                                "MACRO(123)";
        ASSERT_EQUALS("\n$123+$123", OurPreprocessor::expandMacros(filedata));
    }

    void macro_simple18() const {  // (1e-7)
        const char filedata1[] = "#define A (1e-7)\n"
                                 "a=A;";
        ASSERT_EQUALS("\na=$($1e-7);", OurPreprocessor::expandMacros(filedata1));

        const char filedata2[] = "#define A (1E-7)\n"
                                 "a=A;";
        ASSERT_EQUALS("\na=$($1E-7);", OurPreprocessor::expandMacros(filedata2));

        const char filedata3[] = "#define A (1e+7)\n"
                                 "a=A;";
        ASSERT_EQUALS("\na=$($1e+7);", OurPreprocessor::expandMacros(filedata3));

        const char filedata4[] = "#define A (1.e+7)\n"
                                 "a=A;";
        ASSERT_EQUALS("\na=$($1.e+7);", OurPreprocessor::expandMacros(filedata4));

        const char filedata5[] = "#define A (1.7f)\n"
                                 "a=A;";
        ASSERT_EQUALS("\na=$($1.7f);", OurPreprocessor::expandMacros(filedata5));

        const char filedata6[] = "#define A (.1)\n"
                                 "a=A;";
        ASSERT_EQUALS("\na=$($.1);", OurPreprocessor::expandMacros(filedata6));

        const char filedata7[] = "#define A (1.)\n"
                                 "a=A;";
        ASSERT_EQUALS("\na=$($1.);", OurPreprocessor::expandMacros(filedata7));

        const char filedata8[] = "#define A (8.0E+007)\n"
                                 "a=A;";
        ASSERT_EQUALS("\na=$($8.0E+007);", OurPreprocessor::expandMacros(filedata8));
    }

    void macroInMacro1() const {
        {
            const char filedata[] = "#define A(m) long n = m; n++;\n"
                                    "#define B(n) A(n)\n"
                                    "B(0)\n";
            ASSERT_EQUALS("\n\n$$long $n=$0;$n++;\n", OurPreprocessor::expandMacros(filedata));
        }

        {
            const char filedata[] = "#define A B\n"
                                    "#define B 3\n"
                                    "A\n";
            ASSERT_EQUALS("\n\n$$3\n", OurPreprocessor::expandMacros(filedata));
        }

        {
            const char filedata[] = "#define DBG(fmt, args...) printf(fmt, ## args)\n"
                                    "#define D(fmt, args...) DBG(fmt, ## args)\n"
                                    "DBG(\"hello\");\n";
            ASSERT_EQUALS("\n\n$printf(\"hello\");\n", OurPreprocessor::expandMacros(filedata));
        }

        {
            const char filedata[] = "#define DBG(fmt, args...) printf(fmt, ## args)\n"
                                    "#define D(fmt, args...) DBG(fmt, ## args)\n"
                                    "DBG(\"hello: %d\",3);\n";
            ASSERT_EQUALS("\n\n$printf(\"hello: %d\",$3);\n", OurPreprocessor::expandMacros(filedata));
        }

        {
            const char filedata[] = "#define BC(b, c...) 0##b * 0##c\n"
                                    "#define ABC(a, b...) a + BC(b)\n"
                                    "\n"
                                    "ABC(1);\n"
                                    "ABC(2,3);\n"
                                    "ABC(4,5,6);\n";

            ASSERT_EQUALS("\n\n\n$1+$$0*$0;\n$2+$$03*$0;\n$4+$$05*$06;\n", OurPreprocessor::expandMacros(filedata));
        }

        {
            const char filedata[] = "#define A 4\n"
                                    "#define B(a) a,A\n"
                                    "B(2);\n";
            ASSERT_EQUALS("\n\n$2, $4;\n", OurPreprocessor::expandMacros(filedata));
        }

        {
            const char filedata[] = "#define A(x) (x)\n"
                                    "#define B )A(\n"
                                    "#define C )A(\n";
            ASSERT_EQUALS("\n\n\n", OurPreprocessor::expandMacros(filedata));
        }

        {
            const char filedata[] = "#define A(x) (x*2)\n"
                                    "#define B A(\n"
                                    "foo B(i));\n";
            ASSERT_EQUALS("\n\nfoo $$(($i)*$2);\n", OurPreprocessor::expandMacros(filedata));
        }

        {
            const char filedata[] = "#define foo foo\n"
                                    "foo\n";
            ASSERT_EQUALS("\n$foo\n", OurPreprocessor::expandMacros(filedata));
        }

        {
            const char filedata[] =
                "#define B(A1, A2) } while (0)\n"
                "#define A(name) void foo##name() { do { B(1, 2); }\n"
                "A(0)\n"
                "A(1)\n";
            ASSERT_EQUALS("\n\n$void $foo0(){$do{$$}$while($0);}\n$void $foo1(){$do{$$}$while($0);}\n", OurPreprocessor::expandMacros(filedata));
        }

        {
            const char filedata[] =
                "#define B(x) (\n"
                "#define A() B(xx)\n"
                "B(1) A() ) )\n";
            ASSERT_EQUALS("\n\n$( $$( ) )\n", OurPreprocessor::expandMacros(filedata));
        }

        {
            const char filedata[] =
                "#define PTR1 (\n"
                "#define PTR2 PTR1 PTR1\n"
                "int PTR2 PTR2 foo )))) = 0;\n";
            ASSERT_EQUALS("\n\nint $$( $$( $$( $$( foo )))) = 0;\n", OurPreprocessor::expandMacros(filedata));
        }

        {
            const char filedata[] =
                "#define PTR1 (\n"
                "PTR1 PTR1\n";
            ASSERT_EQUALS("\n$( $(\n", OurPreprocessor::expandMacros(filedata));
        }
    }

    void macroInMacro2() const {
        const char filedata[] = "#define A(x) a##x\n"
                                "#define B 0\n"
                                "A(B)\n";
        ASSERT_EQUALS("\n\n$aB\n", OurPreprocessor::expandMacros(filedata));
    }

    void macro_mismatch() const {
        const char filedata[] = "#define AAA(aa,bb) f(aa)\n"
                                "AAA(5);\n";
        ASSERT_EQUALS("\nAAA(5);\n", OurPreprocessor::expandMacros(filedata));
    }

    void macro_linenumbers() const {
        const char filedata[] = "#define AAA(a)\n"
                                "AAA(5\n"
                                "\n"
                                ")\n"
                                "int a;\n";
        ASSERT_EQUALS("\n$"
                      "\n"
                      "\n"
                      "\n"
                      "int a;\n",
                      OurPreprocessor::expandMacros(filedata));
    }

    void macro_nopar() const {
        const char filedata[] = "#define AAA( ) { NULL }\n"
                                "AAA()\n";
        ASSERT_EQUALS("\n${ $NULL }\n", OurPreprocessor::expandMacros(filedata));
    }

    void macro_switchCase() const {
        {
            // Make sure "case 2" doesn't become "case2"
            const char filedata[] = "#define A( b ) "
                                    "switch( a ){ "
                                    "case 2: "
                                    " break; "
                                    "}\n"
                                    "A( 5 );\n";
            ASSERT_EQUALS("\n$switch($a){$case $2:$break;};\n", OurPreprocessor::expandMacros(filedata));
        }

        {
            // Make sure "2 BB" doesn't become "2BB"
            const char filedata[] = "#define A() AA : 2 BB\n"
                                    "A();\n";
            ASSERT_EQUALS("\n$AA : $2 $BB;\n", OurPreprocessor::expandMacros(filedata));
        }

        {
            const char filedata[] = "#define A }\n"
                                    "#define B() A\n"
                                    "#define C( a ) B() break;\n"
                                    "{C( 2 );\n";
            ASSERT_EQUALS("\n\n\n{$$$}$break;;\n", OurPreprocessor::expandMacros(filedata));
        }


        {
            const char filedata[] = "#define A }\n"
                                    "#define B() A\n"
                                    "#define C( a ) B() _break;\n"
                                    "{C( 2 );\n";
            ASSERT_EQUALS("\n\n\n{$$$}$_break;;\n", OurPreprocessor::expandMacros(filedata));
        }


        {
            const char filedata[] = "#define A }\n"
                                    "#define B() A\n"
                                    "#define C( a ) B() 5;\n"
                                    "{C( 2 );\n";
            ASSERT_EQUALS("\n\n\n{$$$}$5;;\n", OurPreprocessor::expandMacros(filedata));
        }
    }

    void macro_NULL() const {
        // Let the tokenizer handle NULL.
        // See ticket #4482 - UB when passing NULL to variadic function
        ASSERT_EQUALS("\n$0", OurPreprocessor::expandMacros("#define null 0\nnull"));
        ASSERT_EQUALS("\nNULL", OurPreprocessor::expandMacros("#define NULL 0\nNULL"));
    }

    void string1() {
        const char filedata[] = "int main()"
                                "{"
                                "    const char *a = \"#define A\n\";"
                                "}\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS(1, static_cast<unsigned int>(actual.size()));
        ASSERT_EQUALS("int main(){ const char *a = \"#define A\n\";}\n", actual[""]);
    }

    void string2() const {
        const char filedata[] = "#define AAA 123\n"
                                "str = \"AAA\"\n";

        // Compare results..
        ASSERT_EQUALS("\nstr = \"AAA\"\n", OurPreprocessor::expandMacros(filedata));
    }

    void string3() const {
        const char filedata[] = "str(\";\");\n";

        // Compare results..
        ASSERT_EQUALS("str(\";\");\n", OurPreprocessor::expandMacros(filedata));
    }


    void preprocessor_undef() {
        {
            const char filedata[] = "#define AAA int a;\n"
                                    "#undef AAA\n"
                                    "#define AAA char b=0;\n"
                                    "AAA\n";

            // Compare results..
            ASSERT_EQUALS("\n\n\n$char $b=$0;\n", OurPreprocessor::expandMacros(filedata));
        }

        {
            // ticket #403
            const char filedata[] = "#define z p[2]\n"
                                    "#undef z\n"
                                    "int z;\n"
                                    "z = 0;\n";

            Preprocessor preprocessor(nullptr, this);
            ASSERT_EQUALS("\n\nint z;\nz = 0;\n", preprocessor.getcode(filedata, "", ""));
        }
    }

    void defdef() const {
        const char filedata[] = "#define AAA 123\n"
                                "#define AAA 456\n"
                                "#define AAA 789\n"
                                "AAA\n";

        // Compare results..
        ASSERT_EQUALS("\n\n\n$789\n", OurPreprocessor::expandMacros(filedata));
    }

    void preprocessor_doublesharp() const {
        // simple testcase without ##
        const char filedata1[] = "#define TEST(var,val) var = val\n"
                                 "TEST(foo,20);\n";
        ASSERT_EQUALS("\n$foo=$20;\n", OurPreprocessor::expandMacros(filedata1));

        // simple testcase with ##
        const char filedata2[] = "#define TEST(var,val) var##_##val = val\n"
                                 "TEST(foo,20);\n";
        ASSERT_EQUALS("\n$foo_20=$20;\n", OurPreprocessor::expandMacros(filedata2));

        // concat macroname
        const char filedata3[] = "#define ABCD 123\n"
                                 "#define A(B) A##B\n"
                                 "A(BCD)\n";
        ASSERT_EQUALS("\n\n$$123\n", OurPreprocessor::expandMacros(filedata3));

        // Ticket #1802 - inner ## must be expanded before outer macro
        const char filedata4[] = "#define A(B) A##B\n"
                                 "#define a(B) A(B)\n"
                                 "a(A(B))\n";
        ASSERT_EQUALS("\n\n$$AAB\n", OurPreprocessor::expandMacros(filedata4));

        // Ticket #1802 - inner ## must be expanded before outer macro
        const char filedata5[] = "#define AB(A,B) A##B\n"
                                 "#define ab(A,B) AB(A,B)\n"
                                 "ab(a,AB(b,c))\n";
        ASSERT_EQUALS("\n\n$$abc\n", OurPreprocessor::expandMacros(filedata5));

        // Ticket #1802
        const char filedata6[] = "#define AB_(A,B) A ## B\n"
                                 "#define AB(A,B) AB_(A,B)\n"
                                 "#define ab(suf) AB(X, AB_(_, suf))\n"
                                 "#define X x\n"
                                 "ab(y)\n";
        ASSERT_EQUALS("\n\n\n\n$$$x_y\n", OurPreprocessor::expandMacros(filedata6));
    }



    void preprocessor_include_in_str() {
        const char filedata[] = "int main()\n"
                                "{\n"
                                "const char *a = \"#include <string>\n\";\n"
                                "return 0;\n"
                                "}\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS(1, static_cast<unsigned int>(actual.size()));
        ASSERT_EQUALS("int main()\n{\nconst char *a = \"#include <string>\n\";\nreturn 0;\n}\n", actual[""]);
    }




    void va_args_1() const {
        const char filedata[] = "#define DBG(fmt...) printf(fmt)\n"
                                "DBG(\"[0x%lx-0x%lx)\", pstart, pend);\n";

        // Preprocess..
        std::string actual = OurPreprocessor::expandMacros(filedata);

        ASSERT_EQUALS("\n$printf(\"[0x%lx-0x%lx)\",$pstart,$pend);\n", actual);
    }

    void va_args_2() const {
        const char filedata[] = "#define DBG(fmt, args...) printf(fmt, ## args)\n"
                                "DBG(\"hello\");\n";

        // Preprocess..
        std::string actual = OurPreprocessor::expandMacros(filedata);

        ASSERT_EQUALS("\n$printf(\"hello\");\n", actual);
    }

    void va_args_3() const {
        const char filedata[] = "#define FRED(...) { fred(__VA_ARGS__); }\n"
                                "FRED(123)\n";
        ASSERT_EQUALS("\n${ $fred($123); }\n", OurPreprocessor::expandMacros(filedata));
    }

    void va_args_4() const {
        const char filedata[] = "#define FRED(name, ...) name (__VA_ARGS__)\n"
                                "FRED(abc, 123)\n";
        ASSERT_EQUALS("\n$abc($123)\n", OurPreprocessor::expandMacros(filedata));
    }



    void multi_character_character() {
        const char filedata[] = "#define FOO 'ABCD'\n"
                                "int main()\n"
                                "{\n"
                                "if( FOO == 0 );\n"
                                "return 0;\n"
                                "}\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS(1, static_cast<unsigned int>(actual.size()));
        ASSERT_EQUALS("\nint main()\n{\nif( $'ABCD' == 0 );\nreturn 0;\n}\n", actual[""]);
    }


    void stringify() const {
        const char filedata[] = "#define STRINGIFY(x) #x\n"
                                "STRINGIFY(abc)\n";

        // expand macros..
        std::string actual = OurPreprocessor::expandMacros(filedata);

        ASSERT_EQUALS("\n$\"abc\"\n", actual);
    }

    void stringify2() const {
        const char filedata[] = "#define A(x) g(#x)\n"
                                "A(abc);\n";

        // expand macros..
        std::string actual = OurPreprocessor::expandMacros(filedata);

        ASSERT_EQUALS("\n$g(\"abc\");\n", actual);
    }

    void stringify3() const {
        const char filedata[] = "#define A(x) g(#x)\n"
                                "A( abc);\n";

        // expand macros..
        std::string actual = OurPreprocessor::expandMacros(filedata);

        ASSERT_EQUALS("\n$g(\"abc\");\n", actual);
    }

    void stringify4() const {
        const char filedata[] = "#define A(x) #x\n"
                                "1 A(\n"
                                "abc\n"
                                ") 2\n";

        // expand macros..
        std::string actual = OurPreprocessor::expandMacros(filedata);

        ASSERT_EQUALS("\n1 $\n\n\"abc\" 2\n", actual);
    }

    void stringify5() const {
        const char filedata[] = "#define A(x) a(#x,x)\n"
                                "A(foo(\"\\\"\"))\n";
        ASSERT_EQUALS("\n$a(\"foo(\\\"\\\\\\\"\\\")\",$foo(\"\\\"\"))\n", OurPreprocessor::expandMacros(filedata));
    }

    void pragma() {
        const char filedata[] = "#pragma once\n"
                                "void f()\n"
                                "{\n"
                                "}\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS(1, static_cast<unsigned int>(actual.size()));
        ASSERT_EQUALS("\nvoid f()\n{\n}\n", actual[""]);
    }

    void pragma_asm_1() {
        const char filedata[] = "#pragma asm\n"
                                "    mov r1, 11\n"
                                "#pragma endasm\n"
                                "aaa\n"
                                "#pragma asm foo\n"
                                "    mov r1, 11\n"
                                "#pragma endasm bar\n"
                                "bbb";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS(1, static_cast<unsigned int>(actual.size()));
        ASSERT_EQUALS("\n\n\naaa\n\n\n\nbbb\n", actual[""]);
    }

    void pragma_asm_2() {
        const char filedata[] = "#pragma asm\n"
                                "    mov @w1, 11\n"
                                "#pragma endasm ( temp=@w1 )\n"
                                "bbb";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS(1, static_cast<unsigned int>(actual.size()));
        ASSERT_EQUALS("\n\nasm(temp);\nbbb\n", actual[""]);
    }

    void pragma_once() {
        const char code[] = "#pragma once\n"
                            "int x";

        Preprocessor preprocessor(nullptr, this);
        const std::list<std::string> includePaths;
        std::map<std::string,std::string> defs;
        std::set<std::string> pragmaOnce;
        preprocessor.handleIncludes(code, "123.h", includePaths, defs, pragmaOnce, std::list<std::string>());
        ASSERT_EQUALS(1U, pragmaOnce.size());
        ASSERT_EQUALS("123.h", *(pragmaOnce.begin()));
    }

    void endifsemicolon() {
        const char filedata[] = "void f() {\n"
                                "#ifdef A\n"
                                "#endif;\n"
                                "}\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS(2, static_cast<unsigned int>(actual.size()));
        const std::string expected("void f() {\n\n\n}\n");
        ASSERT_EQUALS(expected, actual[""]);
        ASSERT_EQUALS(expected, actual["A"]);
    }

    void handle_error() {
        {
            const char filedata[] = "#define A \n"
                                    "#define B don't want to \\\n"
                                    "more text\n"
                                    "void f()\n"
                                    "{\n"
                                    "  char a = 'a'; // '\n"
                                    "}\n";
            const char expected[] = "\n"
                                    "\n"
                                    "\n"
                                    "void f()\n"
                                    "{\n"
                                    "char a = 'a';\n"
                                    "}\n";
            errout.str("");
            // Preprocess => actual result..
            std::istringstream istr(filedata);
            std::map<std::string, std::string> actual;
            Settings settings;
            Preprocessor preprocessor(&settings, this);
            preprocessor.preprocess(istr, actual, "file.c");

            ASSERT_EQUALS(expected, actual[""]);
            ASSERT_EQUALS("", errout.str());
        }
    }

    void missing_doublequote() {
        {
            const char filedata[] = "#define a\n"
                                    "#ifdef 1\n"
                                    "\"\n"
                                    "#endif\n";

            // expand macros..
            errout.str("");
            const std::string actual(OurPreprocessor::expandMacros(filedata, this));

            ASSERT_EQUALS("", actual);
            ASSERT_EQUALS("[file.cpp:3]: (error) No pair for character (\"). Can't process file. File is either invalid or unicode, which is currently not supported.\n", errout.str());
        }

        {
            const char filedata[] = "#file \"abc.h\"\n"
                                    "#define a\n"
                                    "\"\n"
                                    "#endfile\n";

            // expand macros..
            errout.str("");
            const std::string actual(OurPreprocessor::expandMacros(filedata, this));

            ASSERT_EQUALS("", actual);
            ASSERT_EQUALS("[abc.h:2]: (error) No pair for character (\"). Can't process file. File is either invalid or unicode, which is currently not supported.\n", errout.str());
        }

        {
            const char filedata[] = "#file \"abc.h\"\n"
                                    "#define a\n"
                                    "#endfile\n"
                                    "\"\n";

            // expand macros..
            errout.str("");
            const std::string actual(OurPreprocessor::expandMacros(filedata, this));

            ASSERT_EQUALS("", actual);
            ASSERT_EQUALS("[file.cpp:2]: (error) No pair for character (\"). Can't process file. File is either invalid or unicode, which is currently not supported.\n", errout.str());
        }

        {
            const char filedata[] = "#define A 1\n"
                                    "#define B \"\n"
                                    "int a = A;\n";

            // expand macros..
            errout.str("");
            const std::string actual(OurPreprocessor::expandMacros(filedata, this));

            ASSERT_EQUALS("\n\nint a = $1;\n", actual);
            ASSERT_EQUALS("", errout.str());
        }

        {
            const char filedata[] = "void foo()\n"
                                    "{\n"
                                    "\n"
                                    "\n"
                                    "\n"
                                    "int a = 0;\n"
                                    "printf(Text\");\n"
                                    "}\n";

            // expand macros..
            errout.str("");
            OurPreprocessor::expandMacros(filedata, this);

            ASSERT_EQUALS("[file.cpp:7]: (error) No pair for character (\"). Can't process file. File is either invalid or unicode, which is currently not supported.\n", errout.str());
        }
    }

    void unicodeInCode() {
        const std::string filedata("a\xC8");
        std::istringstream istr(filedata);
        errout.str("");
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        preprocessor.read(istr, "test.cpp");
        ASSERT_EQUALS("[test.cpp:1]: (error) The code contains unhandled characters (character code = 0xc8). Checking continues, but do not expect valid results.\n", errout.str());
    }

    void unicodeInComment() {
        const std::string filedata("//\xC8");
        std::istringstream istr(filedata);
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        ASSERT_EQUALS("", preprocessor.read(istr, "test.cpp"));
    }

    void unicodeInString() {
        const std::string filedata("\"\xC8\"");
        std::istringstream istr(filedata);
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        ASSERT_EQUALS(filedata, preprocessor.read(istr, "test.cpp"));
    }


    void define_part_of_func() {
        errout.str("");
        const char filedata[] = "#define A g(\n"
                                "void f() {\n"
                                "  A );\n"
                                "  }\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS(1, static_cast<unsigned int>(actual.size()));
        ASSERT_EQUALS("\nvoid f() {\n$g( );\n}\n", actual[""]);
        ASSERT_EQUALS("", errout.str());
    }

    void conditionalDefine() {
        const char filedata[] = "#ifdef A\n"
                                "#define N 10\n"
                                "#else\n"
                                "#define N 20\n"
                                "#endif\n"
                                "N";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS(2, static_cast<unsigned int>(actual.size()));
        ASSERT_EQUALS("\n\n\n\n\n$20\n", actual[""]);
        ASSERT_EQUALS("\n\n\n\n\n$10\n", actual["A"]);
        ASSERT_EQUALS("", errout.str());
    }


    void multiline_comment() {
        errout.str("");
        const char filedata[] = "#define ABC {// \\\n"
                                "}\n"
                                "void f() ABC }\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS(1, static_cast<unsigned int>(actual.size()));
        ASSERT_EQUALS("\n\nvoid f() ${ }\n", actual[""]);
        ASSERT_EQUALS("", errout.str());
    }

    void macro_parameters() {
        errout.str("");
        const char filedata[] = "#define BC(a, b, c, arg...) \\\n"
                                "AB(a, b, c, ## arg)\n"
                                "\n"
                                "void f()\n"
                                "{\n"
                                "  BC(3);\n"
                                "}\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c", std::list<std::string>());

        // Compare results..
        ASSERT_EQUALS(1, static_cast<unsigned int>(actual.size()));
        ASSERT_EQUALS("", actual[""]);
        ASSERT_EQUALS("[file.c:6]: (error) Syntax error. Not enough parameters for macro 'BC'.\n", errout.str());
    }

    void newline_in_macro() {
        errout.str("");
        const char filedata[] = "#define ABC(str) printf( str )\n"
                                "void f()\n"
                                "{\n"
                                "  ABC(\"\\n\");\n"
                                "}\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c", std::list<std::string>());

        // Compare results..
        ASSERT_EQUALS(1, static_cast<unsigned int>(actual.size()));
        ASSERT_EQUALS("\nvoid f()\n{\n$printf(\"\\n\");\n}\n", actual[""]);
        ASSERT_EQUALS("", errout.str());
    }

    void includes() const {
        {
            std::string src = "#include a.h";
            ASSERT_EQUALS(OurPreprocessor::NoHeader, OurPreprocessor::getHeaderFileName(src));
            ASSERT_EQUALS("", src);
        }

        {
            std::string src = "#include \"b.h\"";
            ASSERT_EQUALS(OurPreprocessor::UserHeader, OurPreprocessor::getHeaderFileName(src));
            ASSERT_EQUALS("b.h", src);
        }

        {
            std::string src = "#include <c.h>";
            ASSERT_EQUALS(OurPreprocessor::SystemHeader, OurPreprocessor::getHeaderFileName(src));
            ASSERT_EQUALS("c.h", src);
        }

        {
            std::string src = "#include \"d/d.h\"";
            ASSERT_EQUALS(OurPreprocessor::UserHeader, OurPreprocessor::getHeaderFileName(src));
            ASSERT_EQUALS("d/d.h", src);
        }

        {
            std::string src = "#include \"e\\e.h\"";
            ASSERT_EQUALS(OurPreprocessor::UserHeader, OurPreprocessor::getHeaderFileName(src));
            ASSERT_EQUALS("e/e.h", src);
        }
    }

    void ifdef_ifdefined() {
        const char filedata[] = "#ifdef ABC\n"
                                "A\n"
                                "#endif\t\n"
                                "#if defined ABC\n"
                                "A\n"
                                "#endif\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS("\n\n\n\n\n\n", actual[""]);
        ASSERT_EQUALS("\nA\n\n\nA\n\n", actual["ABC"]);
        ASSERT_EQUALS(2, static_cast<unsigned int>(actual.size()));
    }

    void define_if1() {
        Preprocessor preprocessor(nullptr, this);

        {
            const char filedata[] = "#define A 0\n"
                                    "#if A\n"
                                    "FOO\n"
                                    "#endif";
            ASSERT_EQUALS("\n\n\n\n", preprocessor.getcode(filedata,"",""));
        }
        {
            const char filedata[] = "#define A 1\n"
                                    "#if A==1\n"
                                    "FOO\n"
                                    "#endif";
            ASSERT_EQUALS("\n\nFOO\n\n", preprocessor.getcode(filedata,"",""));
        }
    }

    void define_if2() {
        const char filedata[] = "#define A 22\n"
                                "#define B A\n"
                                "#if (B==A) || (B==C)\n"
                                "FOO\n"
                                "#endif";
        Preprocessor preprocessor(nullptr, this);
        ASSERT_EQUALS("\n\n\nFOO\n\n", preprocessor.getcode(filedata,"",""));
    }

    void define_if3() {
        const char filedata[] = "#define A 0\n"
                                "#if (A==0)\n"
                                "FOO\n"
                                "#endif";
        Preprocessor preprocessor(nullptr, this);
        ASSERT_EQUALS("\n\nFOO\n\n", preprocessor.getcode(filedata,"",""));
    }

    void define_if4() {
        const char filedata[] = "#define X +123\n"
                                "#if X==123\n"
                                "FOO\n"
                                "#endif";
        Preprocessor preprocessor(nullptr, this);
        ASSERT_EQUALS("\n\nFOO\n\n", preprocessor.getcode(filedata,"",""));
    }

    void define_if5() { // #4516 - #define B (A & 0x00f0)
        {
            const char filedata[] = "#define A 0x0010\n"
                                    "#define B (A & 0x00f0)\n"
                                    "#if B==0x0010\n"
                                    "FOO\n"
                                    "#endif";
            Preprocessor preprocessor(nullptr, this);
            ASSERT_EQUALS("\n\n\nFOO\n\n", preprocessor.getcode(filedata,"",""));
        }
        {
            const char filedata[] = "#define A 0x00f0\n"
                                    "#define B (16)\n"
                                    "#define C (B & A)\n"
                                    "#if C==0x0010\n"
                                    "FOO\n"
                                    "#endif";
            Preprocessor preprocessor(nullptr, this);
            ASSERT_EQUALS("\n\n\n\nFOO\n\n", preprocessor.getcode(filedata,"",""));
        }
        {
            const char filedata[] = "#define A (1+A)\n" // don't hang for recursive macros
                                    "#if A==1\n"
                                    "FOO\n"
                                    "#endif";
            Preprocessor preprocessor(nullptr, this);
            ASSERT_EQUALS("\n\n\n\n", preprocessor.getcode(filedata,"",""));
        }
    }

    void define_if6() { // #4516 - #define B (A?1:-1)
        const char filedata[] = "#ifdef A\n"
                                "#define B (A?1:-1)\n"
                                "#endif\n"
                                "\n"
                                "#if B < 0\n"
                                "123\n"
                                "#endif\n"
                                "\n"
                                "#if B >= 0\n"
                                "456\n"
                                "#endif\n";
        Preprocessor preprocessor(nullptr, this);
        const std::string actualA0 = preprocessor.getcode(filedata, "A=0", "test.c");
        ASSERT_EQUALS(true,  actualA0.find("123") != std::string::npos);
        ASSERT_EQUALS(false, actualA0.find("456") != std::string::npos);
        const std::string actualA1 = preprocessor.getcode(filedata, "A=1", "test.c");
        ASSERT_EQUALS(false, actualA1.find("123") != std::string::npos);
        ASSERT_EQUALS(true,  actualA1.find("456") != std::string::npos);
    }

    void define_ifdef() {
        {
            const char filedata[] = "#define ABC\n"
                                    "#ifndef ABC\n"
                                    "A\n"
                                    "#else\n"
                                    "B\n"
                                    "#endif\n";

            // Preprocess => actual result..
            std::istringstream istr(filedata);
            std::map<std::string, std::string> actual;
            Settings settings;
            Preprocessor preprocessor(&settings, this);
            preprocessor.preprocess(istr, actual, "file.c");

            // Compare results..
            ASSERT_EQUALS("\n\n\n\nB\n\n", actual[""]);
            ASSERT_EQUALS(1, (int)actual.size());
        }

        {
            const char filedata[] = "#define A 1\n"
                                    "#ifdef A\n"
                                    "A\n"
                                    "#endif\n";

            // Preprocess => actual result..
            std::istringstream istr(filedata);
            std::map<std::string, std::string> actual;
            Settings settings;
            Preprocessor preprocessor(&settings, this);
            preprocessor.preprocess(istr, actual, "file.c");

            // Compare results..
            ASSERT_EQUALS("\n\n$1\n\n", actual[""]);
            ASSERT_EQUALS(1, (int)actual.size());
        }

        {
            const char filedata[] = "#define A 1\n"
                                    "#if A==1\n"
                                    "A\n"
                                    "#endif\n";

            // Preprocess => actual result..
            std::istringstream istr(filedata);
            std::map<std::string, std::string> actual;
            Settings settings;
            Preprocessor preprocessor(&settings, this);
            preprocessor.preprocess(istr, actual, "file.c");

            // Compare results..
            ASSERT_EQUALS("\n\n$1\n\n", actual[""]);
            ASSERT_EQUALS(1, (int)actual.size());
        }

        {
            const char filedata[] = "#define A 1\n"
                                    "#if A>0\n"
                                    "A\n"
                                    "#endif\n";

            // Preprocess => actual result..
            std::istringstream istr(filedata);
            std::map<std::string, std::string> actual;
            Settings settings;
            Preprocessor preprocessor(&settings, this);
            preprocessor.preprocess(istr, actual, "file.c");

            // Compare results..
            ASSERT_EQUALS("\n\n$1\n\n", actual[""]);
            ASSERT_EQUALS(1, (int)actual.size());
        }

        {
            const char filedata[] = "#define A 1\n"
                                    "#if 0\n"
                                    "#undef A\n"
                                    "#endif\n"
                                    "A\n";

            // Preprocess => actual result..
            std::istringstream istr(filedata);
            std::map<std::string, std::string> actual;
            Settings settings;
            Preprocessor preprocessor(&settings, this);
            preprocessor.preprocess(istr, actual, "file.c");

            // Compare results..
            ASSERT_EQUALS("\n\n\n\n$1\n", actual[""]);
            ASSERT_EQUALS(1, (int)actual.size());
        }
    }

    void define_ifndef1() {
        const char filedata[] = "#define A(x) (x)\n"
                                "#ifndef A\n"
                                ";\n"
                                "#endif\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS("\n\n\n\n", actual[""]);
        ASSERT_EQUALS(1U, actual.size());
    }

    void define_ifndef2() {
        const char filedata[] = "#ifdef A\n"
                                "#define B char\n"
                                "#endif\n"
                                "#ifndef B\n"
                                "#define B int\n"
                                "#endif\n"
                                "B me;\n";

        // Preprocess => actual result..
        Preprocessor preprocessor(nullptr, this);
        ASSERT_EQUALS("\n\n\n\n\n\n$int me;\n", preprocessor.getcode(filedata, "", "a.cpp"));
        ASSERT_EQUALS("\n\n\n\n\n\n$char me;\n", preprocessor.getcode(filedata, "A", "a.cpp"));
    }

    void ifndef_define() {
        const char filedata[] = "#ifndef A\n"
                                "#define A(x) x\n"
                                "#endif\n"
                                "A(123);";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        ASSERT_EQUALS(1U, actual.size());
        ASSERT_EQUALS("\n\n\n$123;\n", actual[""]);
    }

    void undef_ifdef() {
        const char filedata[] = "#undef A\n"
                                "#ifdef A\n"
                                "123\n"
                                "#endif\n";

        // Preprocess => actual result..
        Preprocessor preprocessor(nullptr, this);
        ASSERT_EQUALS("\n\n\n\n", preprocessor.getcode(filedata, "", "a.cpp"));
        ASSERT_EQUALS("\n\n\n\n", preprocessor.getcode(filedata, "A", "a.cpp"));
    }

    void redundant_config() {
        const char filedata[] = "int main() {\n"
                                "#ifdef FOO\n"
                                "#ifdef BAR\n"
                                "    std::cout << 1;\n"
                                "#endif\n"
                                "#endif\n"
                                "\n"
                                "#ifdef BAR\n"
                                "#ifdef FOO\n"
                                "    std::cout << 2;\n"
                                "#endif\n"
                                "#endif\n"
                                "}\n";


        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS(4, (int)actual.size());
        ASSERT(actual.find("") != actual.end());
        ASSERT(actual.find("BAR") != actual.end());
        ASSERT(actual.find("FOO") != actual.end());
        ASSERT(actual.find("BAR;FOO") != actual.end());
    }


    void endfile() {
        const char filedata[] = "char a[] = \"#endfile\";\n"
                                "char b[] = \"#endfile\";\n"
                                "#include \"notfound.h\"\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS("char a[] = \"#endfile\";\nchar b[] = \"#endfile\";\n\n", actual[""]);
        ASSERT_EQUALS(1, (int)actual.size());
    }

    void dup_defines() {
        const char filedata[] = "#ifdef A\n"
                                "#define B\n"
                                "#if defined(B) && defined(A)\n"
                                "a\n"
                                "#else\n"
                                "b\n"
                                "#endif\n"
                                "#endif\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        // B will always be defined if A is defined; the following test
        // cases should be fixed whenever this other bug is fixed
        TODO_ASSERT_EQUALS(2,
                           3, static_cast<unsigned int>(actual.size()));

        if (actual.find("A") == actual.end()) {
            ASSERT_EQUALS("A is checked", "failed");
        } else {
            ASSERT_EQUALS("A is checked", "A is checked");
        }

        if (actual.find("A;A;B") != actual.end()) {
            ASSERT_EQUALS("A;A;B is NOT checked", "failed");
        } else {
            ASSERT_EQUALS("A;A;B is NOT checked", "A;A;B is NOT checked");
        }
    }

    void testPreprocessorRead1() {
        const std::string filedata("/*\n*/ # /*\n*/ defi\\\nne FO\\\nO 10\\\n20");
        std::istringstream istr(filedata);
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        ASSERT_EQUALS("#define FOO 1020", preprocessor.read(istr, "test.cpp"));
    }

    void testPreprocessorRead2() {
        const std::string filedata("\"foo\\\\\nbar\"");
        std::istringstream istr(filedata);
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        ASSERT_EQUALS("\"foo\\bar\"", preprocessor.read(istr, "test.cpp"));
    }

    void testPreprocessorRead3() {
        const std::string filedata("#define A \" a  \"\n\" b\"");
        std::istringstream istr(filedata);
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        ASSERT_EQUALS(filedata, preprocessor.read(istr, "test.cpp"));
    }

    void testPreprocessorRead4() {
        {
            // test < \\> < > (unescaped)
            const std::string filedata("#define A \" \\\\\"/*space*/  \" \"");
            std::istringstream istr(filedata);
            Settings settings;
            Preprocessor preprocessor(&settings, this);
            ASSERT_EQUALS("#define A \" \\\\\" \" \"", preprocessor.read(istr, "test.cpp"));
        }

        {
            // test <" \\\"  "> (unescaped)
            const std::string filedata("#define A \" \\\\\\\"  \"");
            std::istringstream istr(filedata);
            Settings settings;
            Preprocessor preprocessor(&settings, this);
            ASSERT_EQUALS("#define A \" \\\\\\\"  \"", preprocessor.read(istr, "test.cpp"));
        }

        {
            // test <" \\\\">  <" "> (unescaped)
            const std::string filedata("#define A \" \\\\\\\\\"/*space*/  \" \"");
            std::istringstream istr(filedata);
            Settings settings;
            Preprocessor preprocessor(&settings, this);
            ASSERT_EQUALS("#define A \" \\\\\\\\\" \" \"", preprocessor.read(istr, "test.cpp"));
        }
    }

    void invalid_define_1() {
        Settings settings;
        Preprocessor preprocessor(&settings, this);

        std::istringstream src("#define =\n");
        std::string processedFile;
        std::list<std::string> cfg;
        std::list<std::string> paths;
        preprocessor.preprocess(src, processedFile, cfg, "", paths); // don't hang
    }

    void invalid_define_2() {  // #4036 - hang
        Settings settings;
        Preprocessor preprocessor(&settings, this);

        std::istringstream src("#define () {(int f(x) }\n");
        std::string processedFile;
        std::list<std::string> cfg;
        std::list<std::string> paths;
        preprocessor.preprocess(src, processedFile, cfg, "", paths); // don't hang
    }

    void missingInclude() {
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        Preprocessor::missingIncludeFlag = false;

        std::istringstream src("#include \"missing.h\"\n");
        std::string processedFile;
        std::list<std::string> cfg;
        std::list<std::string> paths;
        ASSERT_EQUALS(false, Preprocessor::missingIncludeFlag);
        preprocessor.preprocess(src, processedFile, cfg, "test.c", paths);
        ASSERT_EQUALS(true, Preprocessor::missingIncludeFlag);
    }

    void inline_suppression_for_missing_include() {
        Preprocessor::missingIncludeFlag = false;
        Settings settings;
        settings._inlineSuppressions = true;
        settings.addEnabled("all");
        Preprocessor preprocessor(&settings, this);

        std::istringstream src("// cppcheck-suppress missingInclude\n"
                               "#include \"missing.h\"\n"
                               "int x;");
        std::string processedFile;
        std::list<std::string> cfg;
        std::list<std::string> paths;

        // Don't report that the include is missing
        errout.str("");
        preprocessor.preprocess(src, processedFile, cfg, "test.c", paths);
        ASSERT_EQUALS("", errout.str());
        ASSERT_EQUALS(false, Preprocessor::missingIncludeFlag);
    }

    void predefine1() {
        const std::string src("#if defined X || Y\n"
                              "Fred & Wilma\n"
                              "#endif\n");

        Preprocessor preprocessor(nullptr, this);
        std::string actual = preprocessor.getcode(src, "X=1", "test.c");

        ASSERT_EQUALS("\nFred & Wilma\n\n", actual);
    }

    void predefine2() {
        const std::string src("#if defined(X) && Y\n"
                              "Fred & Wilma\n"
                              "#endif\n");
        {
            Preprocessor preprocessor(nullptr, this);
            std::string actual = preprocessor.getcode(src, "X=1", "test.c");
            ASSERT_EQUALS("\n\n\n", actual);
        }

        {
            Preprocessor preprocessor(nullptr, this);
            std::string actual = preprocessor.getcode(src, "X=1;Y=2", "test.c");
            ASSERT_EQUALS("\nFred & Wilma\n\n", actual);
        }
    }

    void predefine3() {
        // #2871 - define in source is not used if -D is used
        const char code[] = "#define X 1\n"
                            "#define Y X\n"
                            "#if (X == Y)\n"
                            "Fred & Wilma\n"
                            "#endif\n";
        Preprocessor preprocessor(nullptr,this);
        const std::string actual = preprocessor.getcode(code, "TEST", "test.c");
        ASSERT_EQUALS("\n\n\nFred & Wilma\n\n", actual);
    }

    void predefine4() {
        // #3577
        const char code[] = "char buf[X];\n";
        Preprocessor preprocessor(nullptr,this);
        const std::string actual = preprocessor.getcode(code, "X=123", "test.c");
        ASSERT_EQUALS("char buf[$123];\n", actual);
    }

    void predefine5() {  // #3737, #5119 - automatically define __cplusplus
        // #3737...
        const char code[] = "#ifdef __cplusplus\n123\n#endif";
        Preprocessor preprocessor(nullptr,this);
        ASSERT_EQUALS("\n\n\n",    preprocessor.getcode(code, "X=123", "test.c"));
        ASSERT_EQUALS("\n123\n\n", preprocessor.getcode(code, "X=123", "test.cpp"));

        // #5119...
        ASSERT_EQUALS(false, Preprocessor::cplusplus(nullptr,"test.c"));
        ASSERT_EQUALS(true, Preprocessor::cplusplus(nullptr,"test.cpp"));

        Settings settings;
        ASSERT_EQUALS(true, Preprocessor::cplusplus(&settings,"test.cpp"));
        settings.userUndefs.insert("__cplusplus");
        ASSERT_EQUALS(false, Preprocessor::cplusplus(&settings,"test.cpp"));
    }

    void predefine6() {  // #3737 - using -D and -f => check all matching configurations
        const char filedata[] = "#ifdef A\n"
                                "1\n"
                                "#else\n"
                                "2\n"
                                "#endif\n"
                                "#ifdef B\n"
                                "3\n"
                                "#else\n"
                                "4\n"
                                "#endif";

        // actual result..
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        std::map<std::string, std::string> defs;
        defs["A"] = "1";
        const std::list<std::string> configs = preprocessor.getcfgs(filedata, "test1.c", defs);

        // Compare actual result with expected result..
        ASSERT_EQUALS(2U, configs.size());
        ASSERT_EQUALS("", configs.front());
        ASSERT_EQUALS("B", configs.back());
    }


    void simplifyCondition() {
        // Ticket #2794
        std::map<std::string, std::string> cfg;
        cfg["C"] = "";
        std::string condition("defined(A) || defined(B) || defined(C)");
        Preprocessor preprocessor(nullptr, this);
        preprocessor.simplifyCondition(cfg, condition, true);
        ASSERT_EQUALS("1", condition);
    }

    void invalidElIf() {
        // #2942 - segfault
        const char code[] = "#elif (){\n";
        Preprocessor preprocessor(nullptr,this);
        const std::string actual = preprocessor.getcode(code, "TEST", "test.c");
        ASSERT_EQUALS("\n", actual);
    }

    void def_handleIncludes() {
        const std::string filePath("test.c");
        const std::list<std::string> includePaths;
        std::map<std::string,std::string> defs;
        Preprocessor preprocessor(nullptr, this);

        // ifdef
        {
            defs.clear();
            defs["A"] = "";
            {
                std::set<std::string> pragmaOnce;
                const std::string code("#ifdef A\n123\n#endif\n");
                const std::string actual(preprocessor.handleIncludes(code,filePath,includePaths,defs,pragmaOnce,std::list<std::string>()));
                ASSERT_EQUALS("\n123\n\n", actual);
            }{
                std::set<std::string> pragmaOnce;
                const std::string code("#ifdef B\n123\n#endif\n");
                const std::string actual(preprocessor.handleIncludes(code,filePath,includePaths,defs,pragmaOnce,std::list<std::string>()));
                ASSERT_EQUALS("\n\n\n", actual);
            }
        }

        // ifndef
        {
            defs.clear();
            defs["A"] = "";
            {
                std::set<std::string> pragmaOnce;
                const std::string code("#ifndef A\n123\n#endif\n");
                const std::string actual(preprocessor.handleIncludes(code,filePath,includePaths,defs,pragmaOnce,std::list<std::string>()));
                ASSERT_EQUALS("\n\n\n", actual);
            }{
                std::set<std::string> pragmaOnce;
                const std::string code("#ifndef B\n123\n#endif\n");
                const std::string actual(preprocessor.handleIncludes(code,filePath,includePaths,defs,pragmaOnce,std::list<std::string>()));
                ASSERT_EQUALS("\n123\n\n", actual);
            }
        }

        // define - ifndef
        {
            std::set<std::string> pragmaOnce;
            defs.clear();
            const std::string code("#ifndef X\n#define X\n123\n#endif\n"
                                   "#ifndef X\n#define X\n123\n#endif\n");
            const std::string actual(preprocessor.handleIncludes(code,filePath,includePaths,defs,pragmaOnce,std::list<std::string>()));
            ASSERT_EQUALS("\n#define X\n123\n\n" "\n\n\n\n", actual);
        }

        // #define => #if
        {
            std::set<std::string> pragmaOnce;
            defs.clear();
            const std::string code("#define X 123\n"
                                   "#if X==123\n"
                                   "456\n"
                                   "#endif\n");
            const std::string actual(preprocessor.handleIncludes(code,filePath,includePaths,defs,pragmaOnce,std::list<std::string>()));
            ASSERT_EQUALS("#define X 123\n\n456\n\n", actual);
        }

        // #elif
        {
            const std::string code("#if defined(A)\n"
                                   "1\n"
                                   "#elif defined(B)\n"
                                   "2\n"
                                   "#elif defined(C)\n"
                                   "3\n"
                                   "#else\n"
                                   "4\n"
                                   "#endif");
            {
                std::set<std::string> pragmaOnce;
                defs.clear();
                defs["A"] = "";
                defs["C"] = "";
                const std::string actual(preprocessor.handleIncludes(code,filePath,includePaths,defs,pragmaOnce,std::list<std::string>()));
                ASSERT_EQUALS("\n1\n\n\n\n\n\n\n\n", actual);
            }

            {
                std::set<std::string> pragmaOnce;
                defs.clear();
                defs["B"] = "";
                const std::string actual(preprocessor.handleIncludes(code,filePath,includePaths,defs,pragmaOnce,std::list<std::string>()));
                ASSERT_EQUALS("\n\n\n2\n\n\n\n\n\n", actual);
            }

            {
                std::set<std::string> pragmaOnce;
                defs.clear();
                const std::string actual(preprocessor.handleIncludes(code,filePath,includePaths,defs,pragmaOnce,std::list<std::string>()));
                ASSERT_EQUALS("\n\n\n\n\n\n\n4\n\n", actual);
            }
        }

        // #endif
        {
            // see also endifsemicolon
            const std::string code("{\n#ifdef X\n#endif;\n}");
            std::set<std::string> pragmaOnce;
            defs.clear();
            defs["Z"] = "";
            const std::string actual(preprocessor.handleIncludes(code,filePath,includePaths,defs,pragmaOnce,std::list<std::string>()));
            ASSERT_EQUALS("{\n\n\n}\n", actual);
        }

        // #undef
        {
            const std::string code("#ifndef X\n"
                                   "#define X\n"
                                   "123\n"
                                   "#endif\n");

            std::set<std::string> pragmaOnce;

            pragmaOnce.clear();
            defs.clear();
            const std::string actual1(preprocessor.handleIncludes(code,filePath,includePaths,defs,pragmaOnce,std::list<std::string>()));

            pragmaOnce.clear();
            defs.clear();
            const std::string actual(preprocessor.handleIncludes(code + "#undef X\n" + code, filePath, includePaths, defs,pragmaOnce,std::list<std::string>()));

            ASSERT_EQUALS(actual1 + "#undef X\n" + actual1, actual);
        }

        // #error
        {
            errout.str("");
            std::set<std::string> pragmaOnce;
            defs.clear();
            const std::string code("#ifndef X\n#error abc\n#endif");
            const std::string actual(preprocessor.handleIncludes(code,filePath,includePaths,defs,pragmaOnce,std::list<std::string>()));
            ASSERT_EQUALS("\n#error abc\n\n", actual);
        }
    }

    void def_missingInclude() {
        const std::list<std::string> includePaths;
        std::map<std::string,std::string> defs;
        defs["AA"] = "";
        Settings settings;
        Preprocessor preprocessor(&settings,this);

        // missing local include
        {
            const std::string code("#include \"missing-include!!.h\"\n");
            std::set<std::string> pragmaOnce;

            pragmaOnce.clear();
            errout.str("");
            settings = Settings();
            preprocessor.handleIncludes(code,"test.c",includePaths,defs,pragmaOnce,std::list<std::string>());
            ASSERT_EQUALS("", errout.str());

            pragmaOnce.clear();
            errout.str("");
            settings.checkConfiguration = true;
            preprocessor.handleIncludes(code,"test.c",includePaths,defs,pragmaOnce,std::list<std::string>());
            ASSERT_EQUALS("[test.c:1]: (information) Include file: \"missing-include!!.h\" not found.\n", errout.str());

            pragmaOnce.clear();
            errout.str("");
            settings.nomsg.addSuppression("missingInclude");
            preprocessor.handleIncludes(code,"test.c",includePaths,defs,pragmaOnce,std::list<std::string>());
            ASSERT_EQUALS("", errout.str());
        }

        // missing system header
        {
            const std::string code("#include <missing-include!!.h>\n");
            std::set<std::string> pragmaOnce;

            pragmaOnce.clear();
            errout.str("");
            settings = Settings();
            preprocessor.handleIncludes(code,"test.c",includePaths,defs,pragmaOnce,std::list<std::string>());
            ASSERT_EQUALS("", errout.str());

            pragmaOnce.clear();
            errout.str("");
            settings.checkConfiguration = true;
            preprocessor.handleIncludes(code,"test.c",includePaths,defs,pragmaOnce,std::list<std::string>());
            ASSERT_EQUALS("[test.c:1]: (information) Include file: <missing-include!!.h> not found. Please note: Cppcheck does not need standard library headers to get proper results.\n", errout.str());

            pragmaOnce.clear();
            errout.str("");
            settings.nomsg.addSuppression("missingIncludeSystem");
            preprocessor.handleIncludes(code,"test.c",includePaths,defs,pragmaOnce,std::list<std::string>());
            ASSERT_EQUALS("", errout.str());

            pragmaOnce.clear();
            errout.str("");
            settings = Settings();
            settings.nomsg.addSuppression("missingInclude");
            preprocessor.handleIncludes(code,"test.c",includePaths,defs,pragmaOnce,std::list<std::string>());
            ASSERT_EQUALS("", errout.str());
        }

        // #3285 - #elif
        {
            const std::string code("#ifdef GNU\n"
                                   "#elif defined(WIN32)\n"
                                   "#include \"missing-include!!.h\"\n"
                                   "#endif");
            defs.clear();
            defs["GNU"] = "";

            std::set<std::string> pragmaOnce;
            errout.str("");
            settings = Settings();
            preprocessor.handleIncludes(code,"test.c",includePaths,defs,pragmaOnce,std::list<std::string>());
            ASSERT_EQUALS("", errout.str());
        }
    }

    void def_handleIncludes_ifelse1() {
        const std::string filePath("test.c");
        const std::list<std::string> includePaths;
        std::map<std::string,std::string> defs;
        Preprocessor preprocessor(nullptr, this);

        // #3405
        {
            std::set<std::string> pragmaOnce;
            defs.clear();
            defs["A"] = "";
            const std::string code("\n#ifndef PAL_UTIL_UTILS_H_\n"
                                   "#define PAL_UTIL_UTILS_H_\n"
                                   "1\n"
                                   "#ifndef USE_BOOST\n"
                                   "2\n"
                                   "#else\n"
                                   "3\n"
                                   "#endif\n"
                                   "4\n"
                                   "#endif\n"
                                   "\n"
                                   "#ifndef PAL_UTIL_UTILS_H_\n"
                                   "#define PAL_UTIL_UTILS_H_\n"
                                   "5\n"
                                   "#ifndef USE_BOOST\n"
                                   "6\n"
                                   "#else\n"
                                   "7\n"
                                   "#endif\n"
                                   "8\n"
                                   "#endif\n"
                                   "\n");
            std::string actual(preprocessor.handleIncludes(code,filePath,includePaths,defs,pragmaOnce,std::list<std::string>()));

            // the 1,2,4 should be in the result
            actual.erase(0, actual.find("1"));
            while (actual.find("\n") != std::string::npos)
                actual.erase(actual.find("\n"),1);
            ASSERT_EQUALS("124", actual);
        }

        // #3418
        {
            std::set<std::string> pragmaOnce;
            defs.clear();
            const char code[] = "#define A 1\n"
                                "#define B A\n"
                                "#if A == B\n"
                                "123\n"
                                "#endif\n";

            std::string actual(preprocessor.handleIncludes(code, filePath, includePaths, defs,pragmaOnce,std::list<std::string>()));
            ASSERT_EQUALS("#define A 1\n#define B A\n\n123\n\n", actual);
        }
    }

    void def_handleIncludes_ifelse2() { // #3651
        const char code[] = "#if defined(A)\n"
                            "\n"
                            "#if defined(B)\n"
                            "#endif\n"
                            "\n"
                            "#elif defined(C)\n"
                            "\n"
                            "#else\n"
                            "\n"
                            "123\n"
                            "\n"
                            "#endif";

        Preprocessor preprocessor(nullptr, this);

        const std::list<std::string> includePaths;
        std::map<std::string,std::string> defs;
        defs["A"] = "1";
        std::set<std::string> pragmaOnce;
        ASSERT_EQUALS(std::string::npos,  // No "123" in the output
                      preprocessor.handleIncludes(code, "test.c", includePaths, defs, pragmaOnce,std::list<std::string>()).find("123"));
    }

    void def_handleIncludes_ifelse3() { // #4865
        const char code[] = "#ifdef A\n"
                            "#if defined(SOMETHING_NOT_DEFINED)\n"
                            "#else\n"
                            "#endif\n"
                            "#else\n"
                            "#endif";

        Settings settings;
        settings.userUndefs.insert("A");
        Preprocessor preprocessor(&settings, this);

        const std::list<std::string> includePaths;
        std::map<std::string,std::string> defs;
        defs["B"] = "1";
        defs["C"] = "1";
        std::set<std::string> pragmaOnce;
        preprocessor.handleIncludes(code, "test.c", includePaths, defs, pragmaOnce, std::list<std::string>()); // don't crash
    }

    void def_valueWithParentheses() {
        // #define should introduce a new symbol regardless of parentheses in the value
        // and regardless of white space in weird places (people do this for some reason).
        const char code[] = "#define A (Fred)\n"
                            "      #       define B (Flintstone)\n"
                            "     #define C (Barney)\n"
                            "\t#\tdefine\tD\t(Rubble)\t\t\t\n";

        const std::string filePath("test.c");
        const std::list<std::string> includePaths;
        std::map<std::string,std::string> defs;
        std::set<std::string> pragmaOnce;
        Preprocessor preprocessor(nullptr, this);

        std::istringstream istr(code);
        const std::string s(preprocessor.read(istr, ""));
        preprocessor.handleIncludes(s, filePath, includePaths, defs, pragmaOnce, std::list<std::string>());

        ASSERT(defs.find("A") != defs.end());
        ASSERT_EQUALS("(Fred)", defs["A"]);

        ASSERT(defs.find("B") != defs.end());
        ASSERT_EQUALS("(Flintstone)", defs["B"]);

        ASSERT(defs.find("C") != defs.end());
        ASSERT_EQUALS("(Barney)", defs["C"]);

        ASSERT(defs.find("D") != defs.end());
        ASSERT_EQUALS("(Rubble)", defs["D"]);
    }

    void undef1() {
        Settings settings;

        const char filedata[] = "#ifdef X\n"
                                "Fred & Wilma\n"
                                "#endif\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        settings.userUndefs.insert("X");

        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS(1U, actual.size());
        ASSERT_EQUALS("\n\n\n", actual[""]);
    }

    void undef2() {
        Settings settings;

        const char filedata[] = "#ifndef X\n"
                                "Fred & Wilma\n"
                                "#endif\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        settings.userUndefs.insert("X");

        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS(1U, actual.size());
        ASSERT_EQUALS("\nFred & Wilma\n\n", actual[""]);
    }

    void undef3() {
        Settings settings;

        const char filedata[] = "#define X\n"
                                "#ifdef X\n"
                                "Fred & Wilma\n"
                                "#endif\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        settings.userUndefs.insert("X"); // User undefs should override internal defines

        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS(1U, actual.size());
        ASSERT_EQUALS("\n\n\n\n", actual[""]);
    }

    void undef4() {
        Settings settings;

        const char filedata[] = "#define X Y\n"
                                "#ifdef X\n"
                                "Fred & Wilma\n"
                                "#endif\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        settings.userUndefs.insert("X"); // User undefs should override internal defines

        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS(1U, actual.size());
        ASSERT_EQUALS("\n\n\n\n", actual[""]);
    }

    void undef5() {
        Settings settings;

        const char filedata[] = "#define X() Y\n"
                                "#ifdef X\n"
                                "Fred & Wilma\n"
                                "#endif\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        settings.userUndefs.insert("X"); // User undefs should override internal defines

        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS(1U, actual.size());
        ASSERT_EQUALS("\n\n\n\n", actual[""]);
    }

    void undef6() {
        Settings settings;

        const char filedata[] = "#define X Y\n"
                                "#ifdef X\n"
                                "Fred & Wilma\n"
                                "#else\n"
                                "Barney & Betty\n"
                                "#endif\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        settings.userUndefs.insert("X"); // User undefs should override internal defines

        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS(1U, actual.size());
        ASSERT_EQUALS("\n\n\n\nBarney & Betty\n\n", actual[""]);
    }

    void undef7() {
        Settings settings;

        const char filedata[] = "#define X XDefined\n"
                                "X;\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        settings.userUndefs.insert("X"); // User undefs should override internal defines

        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS(1U, actual.size());
        TODO_ASSERT_EQUALS("\n;\n","\n$XDefined;\n", actual[""]);
    }

    void undef8() {
        Settings settings;

        const char filedata[] = "#ifdef HAVE_CONFIG_H\n"
                                "#include \"config.h\"\n"
                                "#endif\n"
                                "\n"
                                "void foo();\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        settings.userUndefs.insert("X"); // User undefs should override internal defines
        settings.checkConfiguration = true;
        errout.str("");

        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS("[file.c:2]: (information) Include file: \"config.h\" not found.\n", errout.str());
        ASSERT_EQUALS("\n\n\n\n\n", actual[""]);
    }

    void undef9() {
        Settings settings;

        const char filedata[] = "#define X Y\n"
                                "#ifndef X\n"
                                "Fred & Wilma\n"
                                "#else\n"
                                "Barney & Betty\n"
                                "#endif\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        settings.userUndefs.insert("X"); // User undefs should override internal defines

        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS(1U, actual.size());
        ASSERT_EQUALS("\n\nFred & Wilma\n\n\n\n", actual[""]);
    }

    void undef10() {
        Settings settings;

        const char filedata[] = "#ifndef X\n"
                                "#endif\n"
                                "#ifndef Y\n"
                                "#endif\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        settings.userUndefs.insert("X"); // User undefs should override internal defines

        Preprocessor preprocessor(&settings, this);

        std::string processedFile;
        std::list<std::string> resultConfigurations;
        const std::list<std::string> includePaths;
        preprocessor.preprocess(istr, processedFile, resultConfigurations, "file.c", includePaths);


        // Compare results. Two configurations "" and "Y". No "X".
        ASSERT_EQUALS(2U, resultConfigurations.size());
        ASSERT_EQUALS("", resultConfigurations.front());
        ASSERT_EQUALS("Y", resultConfigurations.back());
    }

    void handleUndef() {
        Settings settings;
        settings.userUndefs.insert("X");
        const Preprocessor preprocessor(&settings, this);
        std::list<std::string> configurations;

        // configurations to keep
        configurations.clear();
        configurations.push_back("XY;");
        configurations.push_back("AX;");
        configurations.push_back("A;XY");
        preprocessor.handleUndef(configurations);
        ASSERT_EQUALS(3U, configurations.size());

        // configurations to remove
        configurations.clear();
        configurations.push_back("X;Y");
        configurations.push_back("X=1;Y");
        configurations.push_back("A;X;B");
        configurations.push_back("A;X=1;B");
        configurations.push_back("A;X");
        configurations.push_back("A;X=1");
        preprocessor.handleUndef(configurations);
        ASSERT_EQUALS(0U, configurations.size());
    }

    void macroChar() const {
        const char filedata[] = "#define X 1\nX\n";
        ASSERT_EQUALS("\n$1\n", OurPreprocessor::expandMacros(filedata,nullptr));
        OurPreprocessor::macroChar = char(1);
        ASSERT_EQUALS("\n" + std::string(char(1),1U) + "1\n", OurPreprocessor::expandMacros(filedata,nullptr));
        OurPreprocessor::macroChar = '$';
    }

    void validateCfg() {
        Settings settings;
        Preprocessor preprocessor(&settings, this);

        ASSERT_EQUALS(true, preprocessor.validateCfg("", "X=42"));  // don't hang when parsing cfg
        ASSERT_EQUALS(false, preprocessor.validateCfg("int y=Y;", "X=42;Y"));
        ASSERT_EQUALS(false, preprocessor.validateCfg("int x=X;", "X"));
        ASSERT_EQUALS(false, preprocessor.validateCfg("X=1;", "X"));
        ASSERT_EQUALS(true, preprocessor.validateCfg("int x=X;", "Y"));
        ASSERT_EQUALS(true, preprocessor.validateCfg("FOO_DEBUG()", "DEBUG"));
        ASSERT_EQUALS(true, preprocessor.validateCfg("\"DEBUG()\"", "DEBUG"));
        ASSERT_EQUALS(true, preprocessor.validateCfg("\"\\\"DEBUG()\"", "DEBUG"));
        ASSERT_EQUALS(false, preprocessor.validateCfg("\"DEBUG()\" DEBUG", "DEBUG"));
        ASSERT_EQUALS(true, preprocessor.validateCfg("#undef DEBUG", "DEBUG"));

        // #4301:
        // #ifdef A
        // int a = A;  // <- using macro. must use -D so "A" will get a proper value
        errout.str("");
        settings.addEnabled("all");
        preprocessor.setFile0("test.c");
        ASSERT_EQUALS(false, preprocessor.validateCfg("int a=A;", "A"));
        ASSERT_EQUALS("[test.c:1]: (information) Skipping configuration 'A' since the value of 'A' is unknown. Use -D if you want to check it. You can use -U to skip it explicitly.\n", errout.str());

        // #4949:
        // #ifdef A
        // a |= A;  // <- using macro. must use -D so "A" will get a proper value
        errout.str("");
        Settings settings1;
        settings = settings1;
        ASSERT_EQUALS("", preprocessor.getcode("if (x) a|=A;", "A", "test.c"));
        ASSERT_EQUALS("", errout.str());
        settings.addEnabled("information");
        ASSERT_EQUALS("", preprocessor.getcode("if (x) a|=A;", "A", "test.c"));
        ASSERT_EQUALS("[test.c:1]: (information) Skipping configuration 'A' since the value of 'A' is unknown. Use -D if you want to check it. You can use -U to skip it explicitly.\n", errout.str());
    }

    void if_sizeof() { // #4071
        static const char* code = "#if sizeof(unsigned short) == 2\n"
                                  "Fred & Wilma\n"
                                  "#elif sizeof(unsigned short) == 4\n"
                                  "Fred & Wilma\n"
                                  "#else\n"
                                  "#endif";

        Settings settings;
        Preprocessor preprocessor(&settings, this);
        std::istringstream istr(code);
        std::map<std::string, std::string> actual;
        preprocessor.preprocess(istr, actual, "file.c");
        ASSERT_EQUALS("\nFred & Wilma\n\n\n\n\n", actual[""]);
    }

    void double_include() {
        const char code[] = "int x";

        Preprocessor preprocessor(nullptr, this);
        std::list<std::string> includePaths;
        includePaths.push_back(".");
        includePaths.push_back(".");
        std::map<std::string,std::string> defs;
        std::set<std::string> pragmaOnce;
        preprocessor.handleIncludes(code, "123.h", includePaths, defs, pragmaOnce, std::list<std::string>());
    }

    void invalid_ifs() {
        const char filedata[] = "#ifdef\n"
                                "#endif\n"
                                "#ifdef !\n"
                                "#endif\n"
                                "#if defined\n"
                                "#endif\n"
                                "#define f(x) x\n"
                                "#if f(2\n"
                                "#endif\n"
                                "int x;\n";

        // Preprocess => don't crash..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Settings settings;
        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

    }
};

REGISTER_TEST(TestPreprocessor)
