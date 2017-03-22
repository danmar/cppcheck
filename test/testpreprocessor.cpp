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


// The preprocessor that Cppcheck uses is a bit special. Instead of generating
// the code for a known configuration, it generates the code for each configuration.


#include "testsuite.h"
#include "preprocessor.h"
#include "tokenize.h"
#include "token.h"
#include "settings.h"
#include "simplecpp.h"

#include <cstring>
#include <map>
#include <string>
#include <set>


class TestPreprocessor : public TestFixture {
public:
    TestPreprocessor()
        : TestFixture("TestPreprocessor")
        , preprocessor0(settings0, this) {
    }

    class OurPreprocessor : public Preprocessor {
    public:

        static std::string expandMacros(const char code[], ErrorLogger *errorLogger = 0) {
            std::istringstream istr(code);
            simplecpp::OutputList outputList;
            std::vector<std::string> files;
            const simplecpp::TokenList tokens1 = simplecpp::TokenList(istr, files, "file.cpp", &outputList);
            std::map<std::string, simplecpp::TokenList*> filedata;
            simplecpp::TokenList tokens2(files);
            simplecpp::preprocess(tokens2, tokens1, files, filedata, simplecpp::DUI(), &outputList);

            if (errorLogger) {
                static Settings settings;
                Preprocessor p(settings, errorLogger);
                p.reportOutput(outputList, true);
            }

            return tokens2.stringify();
        }
    };

private:
    Settings settings0;
    Preprocessor preprocessor0;

    void run() {

        // The bug that started the whole work with the new preprocessor
        TEST_CASE(Bug2190219);

        TEST_CASE(error1); // #error => don't extract any code
        TEST_CASE(error3);
        TEST_CASE(error4);  // #2919 - wrong filename is reported
        TEST_CASE(error5);
        TEST_CASE(error6);

        TEST_CASE(setPlatformInfo);

        // Handling include guards (don't create extra configuration for it)
        TEST_CASE(includeguard1);
        TEST_CASE(includeguard2);

        TEST_CASE(if0);
        TEST_CASE(if1);

        TEST_CASE(elif);

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

        TEST_CASE(if_or_1);
        TEST_CASE(if_or_2);

        TEST_CASE(if_macro_eq_macro); // #3536
        TEST_CASE(ticket_3675);
        TEST_CASE(ticket_3699);
        TEST_CASE(ticket_4922); // #4922

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
        TEST_CASE(macro_linenumbers);
        TEST_CASE(macro_nopar);
        TEST_CASE(macro_incdec);  // separate ++ and -- with space when expanding such macro: '#define M(X)  A-X'
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
        //TEST_CASE(va_args_2); invalid code
        TEST_CASE(va_args_3);
        TEST_CASE(va_args_4);
        TEST_CASE(va_args_5);
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
        TEST_CASE(endifsemicolon);
        TEST_CASE(missing_doublequote);
        TEST_CASE(handle_error);
        TEST_CASE(dup_defines);

        TEST_CASE(define_part_of_func);
        TEST_CASE(conditionalDefine);
        TEST_CASE(macro_parameters);
        TEST_CASE(newline_in_macro);
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

        TEST_CASE(invalid_define_1); // #2605 - hang for: '#define ='
        TEST_CASE(invalid_define_2); // #4036 - hang for: '#define () {(int f(x) }'

        // inline suppression, missingInclude
        TEST_CASE(inline_suppression_for_missing_include);

        // Using -D to predefine symbols
        TEST_CASE(predefine1);
        TEST_CASE(predefine2);
        TEST_CASE(predefine3);
        TEST_CASE(predefine4);
        TEST_CASE(predefine5);  // automatically define __cplusplus

        TEST_CASE(invalidElIf); // #2942 segfault

        // Preprocessor::getConfigs
        TEST_CASE(getConfigs1);
        TEST_CASE(getConfigs2);
        TEST_CASE(getConfigs3);
        TEST_CASE(getConfigs4);
        TEST_CASE(getConfigs5);
        TEST_CASE(getConfigs7);
        TEST_CASE(getConfigs7a);
        TEST_CASE(getConfigs7b);
        TEST_CASE(getConfigs7c);
        TEST_CASE(getConfigs7d);
        TEST_CASE(getConfigs7e);
        TEST_CASE(getConfigs8);  // #if A==1  => cfg: A=1
        TEST_CASE(getConfigs10); // #5139
        TEST_CASE(getConfigsError);

        TEST_CASE(getConfigsD1);

        TEST_CASE(getConfigsU1);
        TEST_CASE(getConfigsU2);
        TEST_CASE(getConfigsU3);
        TEST_CASE(getConfigsU4);
        TEST_CASE(getConfigsU5);
        TEST_CASE(getConfigsU6);
        TEST_CASE(getConfigsU7);

        TEST_CASE(validateCfg);

        TEST_CASE(if_sizeof);

        TEST_CASE(invalid_ifs); // #5909

        TEST_CASE(garbage);

        TEST_CASE(wrongPathOnErrorDirective);

        TEST_CASE(testDirectiveIncludeTypes);
        TEST_CASE(testDirectiveIncludeLocations);
        TEST_CASE(testDirectiveIncludeComments);

        TEST_CASE(testSameLine);  // #7912
    }

    void preprocess(const char* code, std::map<std::string, std::string>& actual, const char filename[] = "file.c") {
        errout.str("");
        std::istringstream istr(code);
        simplecpp::OutputList outputList;
        std::vector<std::string> files;
        simplecpp::TokenList tokens(istr, files, filename, &outputList);
        tokens.removeComments();
        const std::set<std::string> configs(preprocessor0.getConfigs(tokens));
        preprocessor0.setDirectives(tokens);
        for (std::set<std::string>::const_iterator it = configs.begin(); it != configs.end(); ++it) {
            try {
                const std::string &cfgcode = preprocessor0.getcode(tokens, *it, files, std::string(code).find("#file") != std::string::npos);
                actual[*it] = cfgcode;
            } catch (...) {
            }
        }
    }

    std::string getConfigsStr(const char filedata[], const char *arg=NULL) {
        Settings settings;
        if (arg && std::strncmp(arg,"-D",2)==0)
            settings.userDefines = arg + 2;
        if (arg && std::strncmp(arg,"-U",2)==0)
            settings.userUndefs.insert(arg+2);
        Preprocessor preprocessor(settings, this);
        std::vector<std::string> files;
        std::istringstream istr(filedata);
        simplecpp::TokenList tokens(istr,files);
        tokens.removeComments();
        const std::set<std::string> configs = preprocessor.getConfigs(tokens);
        std::string ret;
        for (std::set<std::string>::const_iterator it = configs.begin(); it != configs.end(); ++it)
            ret += *it + '\n';
        return ret;
    }

    void Bug2190219() {
        const char filedata[] = "#ifdef __cplusplus\n"
                                "cpp\n"
                                "#else\n"
                                "c\n"
                                "#endif";

        {
            // Preprocess => actual result..
            std::map<std::string, std::string> actual;
            preprocess(filedata, actual, "file.cpp");

            // Compare results..
            ASSERT_EQUALS(1U, actual.size());
            ASSERT_EQUALS("\ncpp", actual[""]);
        }

        {
            // Ticket #7102 - skip __cplusplus in C code
            // Preprocess => actual result..
            std::map<std::string, std::string> actual;
            preprocess(filedata, actual, "file.c");

            // Compare results..
            ASSERT_EQUALS(1U, actual.size());
            ASSERT_EQUALS("\n\n\nc", actual[""]);
        }
    }

    void error1() {
        const char filedata[] = "#ifdef A\n"
                                ";\n"
                                "#else\n"
                                "#error abcd\n"
                                "#endif\n";
        ASSERT_EQUALS("\nA\n", getConfigsStr(filedata));
    }

    void error3() {
        errout.str("");
        Settings settings;
        settings.userDefines = "__cplusplus";
        Preprocessor preprocessor(settings, this);
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
            Preprocessor preprocessor(settings, this);
            const std::string code("#file \"ab.h\"\n#error hello world!\n#endfile");
            preprocessor.getcode(code, "TEST", "test.c");
            ASSERT_EQUALS("[ab.h:1]: (error) #error hello world!\n", errout.str());
        }

        // After including a file
        {
            errout.str("");
            Settings settings;
            settings.userDefines = "TEST";
            Preprocessor preprocessor(settings, this);
            const std::string code("#file \"ab.h\"\n\n#endfile\n#error aaa");
            preprocessor.getcode(code, "TEST", "test.c");
            ASSERT_EQUALS("[test.c:2]: (error) #error aaa\n", errout.str());
        }
    }

    void error5() {
        errout.str("");
        Settings settings;
        settings.userDefines = "FOO";
        settings.force = true; // No message if --force is given
        Preprocessor preprocessor(settings, this);
        const std::string code("#error hello world!\n");
        preprocessor.getcode(code, "X", "test.c");
        ASSERT_EQUALS("", errout.str());
    }

    void error6() {
        const char filedata[] = "#ifdef A\n"
                                "#else\n"
                                "#error 1\n"
                                "#endif\n"
                                "#ifdef B\n"
                                "#else\n"
                                "#error 2\n"
                                "#endif\n";
        ASSERT_EQUALS("\nA\nA;B\nB\n", getConfigsStr(filedata));
    }

    void setPlatformInfo() {
        Settings settings;
        Preprocessor preprocessor(settings, this);

        // read code with simplecpp..
        const char filedata[] = "#if sizeof(long) == 4\n"
                                "1\n"
                                "#else\n"
                                "2\n"
                                "#endif\n";
        std::istringstream istr(filedata);
        std::vector<std::string> files;
        simplecpp::TokenList tokens(istr, files, "test.c");

        // preprocess code with unix32 platform..
        settings.platform(Settings::PlatformType::Unix32);
        preprocessor.setPlatformInfo(&tokens);
        ASSERT_EQUALS("\n1", preprocessor.getcode(tokens, "", files, false));

        // preprocess code with unix64 platform..
        settings.platform(Settings::PlatformType::Unix64);
        preprocessor.setPlatformInfo(&tokens);
        ASSERT_EQUALS("\n\n\n2", preprocessor.getcode(tokens, "", files, false));
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
        ASSERT_EQUALS("\nABC\n", getConfigsStr(filedata));
    }

    void includeguard2() {
        // Handling include guards..
        const char filedata[] = "#file \"abc.h\"\n"
                                "foo\n"
                                "#ifdef ABC\n"
                                "\n"
                                "#endif\n"
                                "#endfile\n";
        ASSERT_EQUALS("\nABC\n", getConfigsStr(filedata));
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
        std::map<std::string, std::string> actual;
        preprocess(filedata, actual);

        // Expected configurations: "" and "ABC"
        ASSERT_EQUALS(2, static_cast<unsigned int>(actual.size()));
        ASSERT_EQUALS("\n\n\nint main ( ) { }", actual[""]);
        ASSERT_EQUALS("\n#line 1 \"abc.h\"\nclass A { } ;\n#line 4 \"file.c\"\n int main ( ) { }", actual["ABC"]);
    }

    void if0() {
        const char filedata[] = " # if /* comment */  0 // comment\n"
                                "#ifdef WIN32\n"
                                "#endif\n"
                                "#endif\n";
        ASSERT_EQUALS("\n", getConfigsStr(filedata));
    }

    void if1() {
        const char filedata[] = " # if /* comment */  1 // comment\n"
                                "ABC\n"
                                " # endif \n";
        ASSERT_EQUALS("\n", getConfigsStr(filedata));
    }


    void elif() {
        {
            const char filedata[] = "#if DEF1\n"
                                    "ABC\n"
                                    "#elif DEF2\n"
                                    "DEF\n"
                                    "#endif\n";
            ASSERT_EQUALS("\nDEF1\nDEF2\n", getConfigsStr(filedata));
        }

        {
            const char filedata[] = "#if(defined DEF1)\n"
                                    "ABC\n"
                                    "#elif(defined DEF2)\n"
                                    "DEF\n"
                                    "#else\n"
                                    "GHI\n"
                                    "#endif\n";
            ASSERT_EQUALS("\nDEF1\nDEF2\n", getConfigsStr(filedata));
        }
    }

    void if_cond1() {
        const char filedata[] = "#if LIBVER>100\n"
                                "    A\n"
                                "#else\n"
                                "    B\n"
                                "#endif\n";
        TODO_ASSERT_EQUALS("\nLIBVER=101\n", "\n", getConfigsStr(filedata));
    }

    void if_cond2() {
        const char filedata[] = "#ifdef A\n"
                                "a\n"
                                "#endif\n"
                                "#if defined(A) && defined(B)\n"
                                "ab\n"
                                "#endif\n";
        ASSERT_EQUALS("\nA\nA;B\n", getConfigsStr(filedata));

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
        TODO_ASSERT_EQUALS("\nA;B\n", "\nA\nB\n", getConfigsStr(filedata));
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
        TODO_ASSERT_EQUALS("\nA\nA;B\n", "\nA\nB\n", getConfigsStr(filedata));
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
        ASSERT_EQUALS("\nA\nA;B\nB\n", getConfigsStr(filedata));
    }

    void if_cond2e() {
        const char filedata[] = "#if !defined(A)\n"
                                "!a\n"
                                "#elif !defined(B)\n"
                                "!b\n"
                                "#endif\n";
        TODO_ASSERT_EQUALS("\nA\nA;B", "\n", getConfigsStr(filedata));
    }

    void if_cond3() {
        const char filedata[] = "#ifdef A\n"
                                "a\n"
                                "#if defined(B) && defined(C)\n"
                                "abc\n"
                                "#endif\n"
                                "#endif\n";
        ASSERT_EQUALS("\nA\nA;B;C\n", getConfigsStr(filedata));
    }

    void if_cond4() {
        {
            const char filedata[] = "#define A\n"
                                    "#define B\n"
                                    "#if defined A || defined B\n"
                                    "ab\n"
                                    "#endif\n";
            ASSERT_EQUALS("\n", getConfigsStr(filedata));
        }

        {
            const char filedata[] = "#if A\n"
                                    "{\n"
                                    "#if (defined(B))\n"
                                    "foo();\n"
                                    "#endif\n"
                                    "}\n"
                                    "#endif\n";
            ASSERT_EQUALS("\nA\nA;B\n", getConfigsStr(filedata));
        }

        {
            const char filedata[] = "#define A\n"
                                    "#define B\n"
                                    "#if (defined A) || defined (B)\n"
                                    "ab\n"
                                    "#endif\n";
            ASSERT_EQUALS("\n", getConfigsStr(filedata));
        }

        {
            const char filedata[] = "#if (A)\n"
                                    "foo();\n"
                                    "#endif\n";
            ASSERT_EQUALS("\nA\n", getConfigsStr(filedata));
        }

        {
            const char filedata[] = "#if! A\n"
                                    "foo();\n"
                                    "#endif\n";
            ASSERT_EQUALS("\n", getConfigsStr(filedata));
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
        ASSERT_EQUALS("\nA;B\n", getConfigsStr(filedata));
    }

    void if_cond6() {
        const char filedata[] = "\n"
                                "#if defined(A) && defined(B))\n"
                                "#endif\n";
        ASSERT_EQUALS("\nA;B\n", getConfigsStr(filedata));
    }

    void if_cond8() {
        const char filedata[] = "#if defined(A) + defined(B) + defined(C) != 1\n"
                                "#endif\n";
        TODO_ASSERT_EQUALS("\nA\n", "\nA;B;C\n", getConfigsStr(filedata));
    }


    void if_cond9() {
        const char filedata[] = "#if !defined _A\n"
                                "abc\n"
                                "#endif\n";
        ASSERT_EQUALS("\n", getConfigsStr(filedata));
    }

    void if_cond10() {
        const char filedata[] = "#if !defined(a) && !defined(b)\n"
                                "#if defined(and)\n"
                                "#endif\n"
                                "#endif\n";

        // Preprocess => don't crash..
        std::map<std::string, std::string> actual;
        preprocess(filedata, actual);
    }

    void if_cond11() {
        const char filedata[] = "#if defined(L_fixunssfdi) && LIBGCC2_HAS_SF_MODE\n"
                                "#if LIBGCC2_HAS_DF_MODE\n"
                                "#elif FLT_MANT_DIG < W_TYPE_SIZE\n"
                                "#endif\n"
                                "#endif\n";
        std::map<std::string, std::string> actual;
        preprocess(filedata, actual);
        ASSERT_EQUALS("", errout.str());
    }

    void if_cond12() {
        const char filedata[] = "#define A (1)\n"
                                "#if A == 1\n"
                                ";\n"
                                "#endif\n";
        ASSERT_EQUALS("\n", getConfigsStr(filedata));
    }

    void if_cond13() {
        const char filedata[] = "#if ('A' == 0x41)\n"
                                "123\n"
                                "#endif\n";
        ASSERT_EQUALS("\n", getConfigsStr(filedata));
    }

    void if_cond14() {
        const char filedata[] = "#if !(A)\n"
                                "123\n"
                                "#endif\n";
        ASSERT_EQUALS("\n", getConfigsStr(filedata));
    }



    void if_or_1() {
        const char filedata[] = "#if defined(DEF_10) || defined(DEF_11)\n"
                                "a1;\n"
                                "#endif\n";
        ASSERT_EQUALS("\nDEF_10;DEF_11\n", getConfigsStr(filedata));
    }

    void if_or_2() {
        const char filedata[] = "#if X || Y\n"
                                "a1;\n"
                                "#endif\n";
        TODO_ASSERT_EQUALS("\nX;Y\n", "\n", getConfigsStr(filedata));
    }

    void if_macro_eq_macro() {
        const char *code = "#define A B\n"
                           "#define B 1\n"
                           "#define C 1\n"
                           "#if A == C\n"
                           "Wilma\n"
                           "#else\n"
                           "Betty\n"
                           "#endif\n";
        ASSERT_EQUALS("\n", getConfigsStr(code));
    }

    void ticket_3675() {
        const char* code = "#ifdef YYSTACKSIZE\n"
                           "#define YYMAXDEPTH YYSTACKSIZE\n"
                           "#else\n"
                           "#define YYSTACKSIZE YYMAXDEPTH\n"
                           "#endif\n"
                           "#if YYDEBUG\n"
                           "#endif\n";
        std::map<std::string, std::string> actual;
        preprocess(code, actual);

        // There's nothing to assert. It just needs to not hang.
    }

    void ticket_3699() {
        const char* code = "#define INLINE __forceinline\n"
                           "#define inline __forceinline\n"
                           "#define __forceinline inline\n"
                           "#if !defined(_WIN32)\n"
                           "#endif\n"
                           "INLINE inline __forceinline\n";
        std::map<std::string, std::string> actual;
        preprocess(code, actual);

        // First, it must not hang. Second, inline must becomes inline, and __forceinline must become __forceinline.
        ASSERT_EQUALS("\n\n\n\n\n$__forceinline $inline $__forceinline", actual[""]);
    }

    void ticket_4922() { // #4922
        const char* code = "__asm__ \n"
                           "{ int extern __value) 0; (double return (\"\" } extern\n"
                           "__typeof __finite (__finite) __finite __inline \"__GI___finite\");";
        std::map<std::string, std::string> actual;
        preprocess(code, actual);
    }

    void macro_simple1() const {
        {
            const char filedata[] = "#define AAA(aa) f(aa)\n"
                                    "AAA(5);\n";
            ASSERT_EQUALS("\nf ( 5 ) ;", OurPreprocessor::expandMacros(filedata));
        }

        {
            const char filedata[] = "#define AAA(aa) f(aa)\n"
                                    "AAA (5);\n";
            ASSERT_EQUALS("\nf ( 5 ) ;", OurPreprocessor::expandMacros(filedata));
        }
    }

    void macro_simple2() const {
        const char filedata[] = "#define min(x,y) x<y?x:y\n"
                                "min(a(),b());\n";
        ASSERT_EQUALS("\na ( ) < b ( ) ? a ( ) : b ( ) ;", OurPreprocessor::expandMacros(filedata));
    }

    void macro_simple3() const {
        const char filedata[] = "#define A 4\n"
                                "A AA\n";
        ASSERT_EQUALS("\n4 AA", OurPreprocessor::expandMacros(filedata));
    }

    void macro_simple4() const {
        const char filedata[] = "#define TEMP_1 if( temp > 0 ) return 1;\n"
                                "TEMP_1\n";
        ASSERT_EQUALS("\nif ( temp > 0 ) return 1 ;", OurPreprocessor::expandMacros(filedata));
    }

    void macro_simple5() const {
        const char filedata[] = "#define ABC if( temp > 0 ) return 1;\n"
                                "\n"
                                "void foo()\n"
                                "{\n"
                                "    int temp = 0;\n"
                                "    ABC\n"
                                "}\n";
        ASSERT_EQUALS("\n\nvoid foo ( )\n{\nint temp = 0 ;\nif ( temp > 0 ) return 1 ;\n}", OurPreprocessor::expandMacros(filedata));
    }

    void macro_simple6() const {
        const char filedata[] = "#define ABC (a+b+c)\n"
                                "ABC\n";
        ASSERT_EQUALS("\n( a + b + c )", OurPreprocessor::expandMacros(filedata));
    }

    void macro_simple7() const {
        const char filedata[] = "#define ABC(str) str\n"
                                "ABC(\"(\")\n";
        ASSERT_EQUALS("\n\"(\"", OurPreprocessor::expandMacros(filedata));
    }

    void macro_simple8() const {
        const char filedata[] = "#define ABC 123\n"
                                "#define ABCD 1234\n"
                                "ABC ABCD\n";
        ASSERT_EQUALS("\n\n123 1234", OurPreprocessor::expandMacros(filedata));
    }

    void macro_simple9() const {
        const char filedata[] = "#define ABC(a) f(a)\n"
                                "ABC( \"\\\"\" );\n"
                                "ABC( \"g\" );\n";
        ASSERT_EQUALS("\nf ( \"\\\"\" ) ;\nf ( \"g\" ) ;", OurPreprocessor::expandMacros(filedata));
    }

    void macro_simple10() const {
        const char filedata[] = "#define ABC(t) t x\n"
                                "ABC(unsigned long);\n";
        ASSERT_EQUALS("\nunsigned long x ;", OurPreprocessor::expandMacros(filedata));
    }

    void macro_simple11() const {
        const char filedata[] = "#define ABC(x) delete x\n"
                                "ABC(a);\n";
        ASSERT_EQUALS("\ndelete a ;", OurPreprocessor::expandMacros(filedata));
    }

    void macro_simple12() const {
        const char filedata[] = "#define AB ab.AB\n"
                                "AB.CD\n";
        ASSERT_EQUALS("\nab . AB . CD", OurPreprocessor::expandMacros(filedata));
    }

    void macro_simple13() const {
        const char filedata[] = "#define TRACE(x)\n"
                                "TRACE(;if(a))\n";
        ASSERT_EQUALS("", OurPreprocessor::expandMacros(filedata));
    }

    void macro_simple14() const {
        const char filedata[] = "#define A \"  a  \"\n"
                                "printf(A);\n";
        ASSERT_EQUALS("\nprintf ( \"  a  \" ) ;", OurPreprocessor::expandMacros(filedata));
    }

    void macro_simple15() const {
        const char filedata[] = "#define FOO\"foo\"\n"
                                "FOO\n";
        ASSERT_EQUALS("\n\"foo\"", OurPreprocessor::expandMacros(filedata));
    }

    void macro_simple16() const {  // # 4703
        const char filedata[] = "#define MACRO( A, B, C ) class A##B##C##Creator {};\n"
                                "MACRO( B\t, U , G )";
        ASSERT_EQUALS("\nclass BUGCreator { } ;", OurPreprocessor::expandMacros(filedata));
    }

    void macro_simple17() const {  // # 5074 - the Token::isExpandedMacro() doesn't always indicate properly if token comes from macro
        // It would probably be OK if the generated code was
        // "\n123+$123" since the first 123 comes from the source code
        const char filedata[] = "#define MACRO(A) A+123\n"
                                "MACRO(123)";
        ASSERT_EQUALS("\n123 + 123", OurPreprocessor::expandMacros(filedata));
    }

    void macro_simple18() const {  // (1e-7)
        const char filedata1[] = "#define A (1e-7)\n"
                                 "a=A;";
        ASSERT_EQUALS("\na = ( 1e-7 ) ;", OurPreprocessor::expandMacros(filedata1));

        const char filedata2[] = "#define A (1E-7)\n"
                                 "a=A;";
        ASSERT_EQUALS("\na = ( 1E-7 ) ;", OurPreprocessor::expandMacros(filedata2));

        const char filedata3[] = "#define A (1e+7)\n"
                                 "a=A;";
        ASSERT_EQUALS("\na = ( 1e+7 ) ;", OurPreprocessor::expandMacros(filedata3));

        const char filedata4[] = "#define A (1.e+7)\n"
                                 "a=A;";
        ASSERT_EQUALS("\na = ( 1.e+7 ) ;", OurPreprocessor::expandMacros(filedata4));

        const char filedata5[] = "#define A (1.7f)\n"
                                 "a=A;";
        ASSERT_EQUALS("\na = ( 1.7f ) ;", OurPreprocessor::expandMacros(filedata5));

        const char filedata6[] = "#define A (.1)\n"
                                 "a=A;";
        ASSERT_EQUALS("\na = ( .1 ) ;", OurPreprocessor::expandMacros(filedata6));

        const char filedata7[] = "#define A (1.)\n"
                                 "a=A;";
        ASSERT_EQUALS("\na = ( 1. ) ;", OurPreprocessor::expandMacros(filedata7));

        const char filedata8[] = "#define A (8.0E+007)\n"
                                 "a=A;";
        ASSERT_EQUALS("\na = ( 8.0E+007 ) ;", OurPreprocessor::expandMacros(filedata8));
    }

    void macroInMacro1() const {
        {
            const char filedata[] = "#define A(m) long n = m; n++;\n"
                                    "#define B(n) A(n)\n"
                                    "B(0)\n";
            ASSERT_EQUALS("\n\nlong n = 0 ; n ++ ;", OurPreprocessor::expandMacros(filedata));
        }

        {
            const char filedata[] = "#define A B\n"
                                    "#define B 3\n"
                                    "A\n";
            ASSERT_EQUALS("\n\n3", OurPreprocessor::expandMacros(filedata));
        }

        {
            /* TODO: What to do here? since there are syntax error simplecpp outputs ""
            const char filedata[] = "#define BC(b, c...) 0##b * 0##c\n"
                                    "#define ABC(a, b...) a + BC(b)\n"
                                    "\n"
                                    "ABC(1);\n" // <- too few parameters
                                    "ABC(2,3);\n"
                                    "ABC(4,5,6);\n";

            ASSERT_EQUALS("\n\n\n1 + 0 * 0;\n2 + 03 * 0;\n4 + 05 * 06;", OurPreprocessor::expandMacros(filedata));
            */
        }

        {
            const char filedata[] = "#define A 4\n"
                                    "#define B(a) a,A\n"
                                    "B(2);\n";
            ASSERT_EQUALS("\n\n2 , 4 ;", OurPreprocessor::expandMacros(filedata));
        }

        {
            const char filedata[] = "#define A(x) (x)\n"
                                    "#define B )A(\n"
                                    "#define C )A(\n";
            ASSERT_EQUALS("", OurPreprocessor::expandMacros(filedata));
        }

        {
            const char filedata[] = "#define A(x) (x*2)\n"
                                    "#define B A(\n"
                                    "foo B(i));\n";
            ASSERT_EQUALS("\n\nfoo ( ( i ) * 2 ) ;", OurPreprocessor::expandMacros(filedata));
        }

        {
            const char filedata[] = "#define foo foo\n"
                                    "foo\n";
            ASSERT_EQUALS("\nfoo", OurPreprocessor::expandMacros(filedata));
        }

        {
            const char filedata[] =
                "#define B(A1, A2) } while (0)\n"
                "#define A(name) void foo##name() { do { B(1, 2); }\n"
                "A(0)\n"
                "A(1)\n";
            ASSERT_EQUALS("\n\nvoid foo0 ( ) { do { } while ( 0 ) ; }\nvoid foo1 ( ) { do { } while ( 0 ) ; }", OurPreprocessor::expandMacros(filedata));
        }

        {
            const char filedata[] =
                "#define B(x) (\n"
                "#define A() B(xx)\n"
                "B(1) A() ) )\n";
            ASSERT_EQUALS("\n\n( ( ) )", OurPreprocessor::expandMacros(filedata));
        }

        {
            const char filedata[] =
                "#define PTR1 (\n"
                "#define PTR2 PTR1 PTR1\n"
                "int PTR2 PTR2 foo )))) = 0;\n";
            ASSERT_EQUALS("\n\nint ( ( ( ( foo ) ) ) ) = 0 ;", OurPreprocessor::expandMacros(filedata));
        }

        {
            const char filedata[] =
                "#define PTR1 (\n"
                "PTR1 PTR1\n";
            ASSERT_EQUALS("\n( (", OurPreprocessor::expandMacros(filedata));
        }
    }

    void macroInMacro2() const {
        const char filedata[] = "#define A(x) a##x\n"
                                "#define B 0\n"
                                "A(B)\n";
        ASSERT_EQUALS("\n\naB", OurPreprocessor::expandMacros(filedata));
    }

    void macro_linenumbers() const {
        const char filedata[] = "#define AAA(a)\n"
                                "AAA(5\n"
                                "\n"
                                ")\n"
                                "int a;\n";
        ASSERT_EQUALS("\n"
                      "\n"
                      "\n"
                      "\n"
                      "int a ;",
                      OurPreprocessor::expandMacros(filedata));
    }

    void macro_nopar() const {
        const char filedata[] = "#define AAA( ) { NULL }\n"
                                "AAA()\n";
        ASSERT_EQUALS("\n{ NULL }", OurPreprocessor::expandMacros(filedata));
    }

    void macro_incdec() const {
        const char filedata[] = "#define M1(X) 1+X\n"
                                "#define M2(X) 2-X\n"
                                "M1(+1) M2(-1)\n";
        ASSERT_EQUALS("\n\n1 + + 1 2 - - 1", OurPreprocessor::expandMacros(filedata));
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
            ASSERT_EQUALS("\nswitch ( a ) { case 2 : break ; } ;", OurPreprocessor::expandMacros(filedata));
        }

        {
            // Make sure "2 BB" doesn't become "2BB"
            const char filedata[] = "#define A() AA : 2 BB\n"
                                    "A();\n";
            ASSERT_EQUALS("\nAA : 2 BB ;", OurPreprocessor::expandMacros(filedata));
        }

        {
            const char filedata[] = "#define A }\n"
                                    "#define B() A\n"
                                    "#define C( a ) B() break;\n"
                                    "{C( 2 );\n";
            ASSERT_EQUALS("\n\n\n{ } break ; ;", OurPreprocessor::expandMacros(filedata));
        }


        {
            const char filedata[] = "#define A }\n"
                                    "#define B() A\n"
                                    "#define C( a ) B() _break;\n"
                                    "{C( 2 );\n";
            ASSERT_EQUALS("\n\n\n{ } _break ; ;", OurPreprocessor::expandMacros(filedata));
        }


        {
            const char filedata[] = "#define A }\n"
                                    "#define B() A\n"
                                    "#define C( a ) B() 5;\n"
                                    "{C( 2 );\n";
            ASSERT_EQUALS("\n\n\n{ } 5 ; ;", OurPreprocessor::expandMacros(filedata));
        }
    }

    void macro_NULL() const {
        // Let the tokenizer handle NULL.
        // See ticket #4482 - UB when passing NULL to variadic function
        ASSERT_EQUALS("\n0", OurPreprocessor::expandMacros("#define null 0\nnull"));
        // TODO ASSERT_EQUALS("\nNULL", OurPreprocessor::expandMacros("#define NULL 0\nNULL"));
    }

    void string1() {
        const char filedata[] = "int main()"
                                "{"
                                "    const char *a = \"#define A\";"
                                "}\n";

        // Preprocess => actual result..
        std::map<std::string, std::string> actual;
        preprocess(filedata, actual);

        // Compare results..
        ASSERT_EQUALS(1, static_cast<unsigned int>(actual.size()));
        ASSERT_EQUALS("int main ( ) { const char * a = \"#define A\" ; }", actual[""]);
    }

    void string2() const {
        const char filedata[] = "#define AAA 123\n"
                                "str = \"AAA\"\n";

        // Compare results..
        ASSERT_EQUALS("\nstr = \"AAA\"", OurPreprocessor::expandMacros(filedata));
    }

    void string3() const {
        const char filedata[] = "str(\";\");\n";

        // Compare results..
        ASSERT_EQUALS("str ( \";\" ) ;", OurPreprocessor::expandMacros(filedata));
    }


    void preprocessor_undef() {
        {
            const char filedata[] = "#define AAA int a;\n"
                                    "#undef AAA\n"
                                    "#define AAA char b=0;\n"
                                    "AAA\n";

            // Compare results..
            ASSERT_EQUALS("\n\n\nchar b = 0 ;", OurPreprocessor::expandMacros(filedata));
        }

        {
            // ticket #403
            const char filedata[] = "#define z p[2]\n"
                                    "#undef z\n"
                                    "int z;\n"
                                    "z = 0;\n";
            ASSERT_EQUALS("\n\nint z ;\nz = 0 ;", preprocessor0.getcode(filedata, "", ""));
        }
    }

    void defdef() const {
        const char filedata[] = "#define AAA 123\n"
                                "#define AAA 456\n"
                                "#define AAA 789\n"
                                "AAA\n";

        // Compare results..
        ASSERT_EQUALS("\n\n\n789", OurPreprocessor::expandMacros(filedata));
    }

    void preprocessor_doublesharp() const {
        // simple testcase without ##
        const char filedata1[] = "#define TEST(var,val) var = val\n"
                                 "TEST(foo,20);\n";
        ASSERT_EQUALS("\nfoo = 20 ;", OurPreprocessor::expandMacros(filedata1));

        // simple testcase with ##
        const char filedata2[] = "#define TEST(var,val) var##_##val = val\n"
                                 "TEST(foo,20);\n";
        ASSERT_EQUALS("\nfoo_20 = 20 ;", OurPreprocessor::expandMacros(filedata2));

        // concat macroname
        const char filedata3[] = "#define ABCD 123\n"
                                 "#define A(B) A##B\n"
                                 "A(BCD)\n";
        ASSERT_EQUALS("\n\n123", OurPreprocessor::expandMacros(filedata3));

        // Ticket #1802 - inner ## must be expanded before outer macro
        const char filedata4[] = "#define A(B) A##B\n"
                                 "#define a(B) A(B)\n"
                                 "a(A(B))\n";
        ASSERT_EQUALS("\n\nAAB", OurPreprocessor::expandMacros(filedata4));

        // Ticket #1802 - inner ## must be expanded before outer macro
        const char filedata5[] = "#define AB(A,B) A##B\n"
                                 "#define ab(A,B) AB(A,B)\n"
                                 "ab(a,AB(b,c))\n";
        ASSERT_EQUALS("\n\nabc", OurPreprocessor::expandMacros(filedata5));

        // Ticket #1802
        const char filedata6[] = "#define AB_(A,B) A ## B\n"
                                 "#define AB(A,B) AB_(A,B)\n"
                                 "#define ab(suf) AB(X, AB_(_, suf))\n"
                                 "#define X x\n"
                                 "ab(y)\n";
        ASSERT_EQUALS("\n\n\n\nx_y", OurPreprocessor::expandMacros(filedata6));
    }



    void preprocessor_include_in_str() {
        const char filedata[] = "int main()\n"
                                "{\n"
                                "const char *a = \"#include <string>\";\n"
                                "return 0;\n"
                                "}\n";

        // Preprocess => actual result..
        std::map<std::string, std::string> actual;
        preprocess(filedata, actual);

        // Compare results..
        ASSERT_EQUALS(1, static_cast<unsigned int>(actual.size()));
        ASSERT_EQUALS("int main ( )\n{\nconst char * a = \"#include <string>\" ;\nreturn 0 ;\n}", actual[""]);
    }




    void va_args_1() const {
        const char filedata[] = "#define DBG(fmt...) printf(fmt)\n"
                                "DBG(\"[0x%lx-0x%lx)\", pstart, pend);\n";

        // Preprocess..
        std::string actual = OurPreprocessor::expandMacros(filedata);

        ASSERT_EQUALS("\nprintf ( \"[0x%lx-0x%lx)\" , pstart , pend ) ;", actual);
    }
    /*
        void va_args_2() const {
            const char filedata[] = "#define DBG(fmt, args...) printf(fmt, ## args)\n"
                                    "DBG(\"hello\");\n";

            // Preprocess..
            std::string actual = OurPreprocessor::expandMacros(filedata);

            // invalid code ASSERT_EQUALS("\nprintf ( \"hello\" ) ;", actual);
        }
    */
    void va_args_3() const {
        const char filedata[] = "#define FRED(...) { fred(__VA_ARGS__); }\n"
                                "FRED(123)\n";
        ASSERT_EQUALS("\n{ fred ( 123 ) ; }", OurPreprocessor::expandMacros(filedata));
    }

    void va_args_4() const {
        const char filedata[] = "#define FRED(name, ...) name (__VA_ARGS__)\n"
                                "FRED(abc, 123)\n";
        ASSERT_EQUALS("\nabc ( 123 )", OurPreprocessor::expandMacros(filedata));
    }

    void va_args_5() const {
        const char filedata1[] = "#define A(...) #__VA_ARGS__\n"
                                 "A(123)\n";
        ASSERT_EQUALS("\n\"123\"", OurPreprocessor::expandMacros(filedata1));

        const char filedata2[] = "#define A(X,...) X(#__VA_ARGS__)\n"
                                 "A(f,123)\n";
        ASSERT_EQUALS("\nf ( \"123\" )", OurPreprocessor::expandMacros(filedata2));
    }



    void multi_character_character() {
        const char filedata[] = "#define FOO 'ABCD'\n"
                                "int main()\n"
                                "{\n"
                                "if( FOO == 0 );\n"
                                "return 0;\n"
                                "}\n";

        // Preprocess => actual result..
        std::map<std::string, std::string> actual;
        preprocess(filedata, actual);

        // Compare results..
        ASSERT_EQUALS(1, static_cast<unsigned int>(actual.size()));
        ASSERT_EQUALS("\nint main ( )\n{\nif ( $'ABCD' == 0 ) ;\nreturn 0 ;\n}", actual[""]);
    }


    void stringify() const {
        const char filedata[] = "#define STRINGIFY(x) #x\n"
                                "STRINGIFY(abc)\n";

        // expand macros..
        std::string actual = OurPreprocessor::expandMacros(filedata);

        ASSERT_EQUALS("\n\"abc\"", actual);
    }

    void stringify2() const {
        const char filedata[] = "#define A(x) g(#x)\n"
                                "A(abc);\n";

        // expand macros..
        std::string actual = OurPreprocessor::expandMacros(filedata);

        ASSERT_EQUALS("\ng ( \"abc\" ) ;", actual);
    }

    void stringify3() const {
        const char filedata[] = "#define A(x) g(#x)\n"
                                "A( abc);\n";

        // expand macros..
        std::string actual = OurPreprocessor::expandMacros(filedata);

        ASSERT_EQUALS("\ng ( \"abc\" ) ;", actual);
    }

    void stringify4() const {
        const char filedata[] = "#define A(x) #x\n"
                                "1 A(\n"
                                "abc\n"
                                ") 2\n";

        // expand macros..
        std::string actual = OurPreprocessor::expandMacros(filedata);

        ASSERT_EQUALS("\n1 \"abc\"\n\n2", actual);
    }

    void stringify5() const {
        const char filedata[] = "#define A(x) a(#x,x)\n"
                                "A(foo(\"\\\"\"))\n";
        ASSERT_EQUALS("\na ( \"foo(\\\"\\\\\\\"\\\")\" , foo ( \"\\\"\" ) )", OurPreprocessor::expandMacros(filedata));
    }

    void pragma() {
        const char filedata[] = "#pragma once\n"
                                "void f()\n"
                                "{\n"
                                "}\n";

        // Preprocess => actual result..
        std::map<std::string, std::string> actual;
        preprocess(filedata, actual);

        // Compare results..
        ASSERT_EQUALS(1, static_cast<unsigned int>(actual.size()));
        ASSERT_EQUALS("\nvoid f ( )\n{\n}", actual[""]);
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
        std::map<std::string, std::string> actual;
        preprocess(filedata, actual);

        // Compare results..
        ASSERT_EQUALS(1, static_cast<unsigned int>(actual.size()));
        ASSERT_EQUALS("\nasm();\n\naaa\n\nasm();\n\nbbb", actual[""]);
    }

    void pragma_asm_2() {
        const char filedata[] = "#pragma asm\n"
                                "    mov @w1, 11\n"
                                "#pragma endasm ( temp=@w1 )\n"
                                "bbb";

        // Preprocess => actual result..
        std::map<std::string, std::string> actual;
        preprocess(filedata, actual);

        // Compare results..
        ASSERT_EQUALS(1, static_cast<unsigned int>(actual.size()));
        ASSERT_EQUALS("\nasm();\n\nbbb", actual[""]);
    }

    void endifsemicolon() {
        const char filedata[] = "void f() {\n"
                                "#ifdef A\n"
                                "#endif;\n"
                                "}\n";

        // Preprocess => actual result..
        std::map<std::string, std::string> actual;
        preprocess(filedata, actual);

        // Compare results..
        ASSERT_EQUALS(2, static_cast<unsigned int>(actual.size()));
        const std::string expected("void f ( ) {\n\n\n}");
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

            // Preprocess => actual result..
            std::map<std::string, std::string> actual;
            preprocess(filedata, actual);

            ASSERT_EQUALS("", actual[""]);
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

            ASSERT_EQUALS("", actual);
            ASSERT_EQUALS("[file.cpp:2]: (error) No pair for character (\"). Can't process file. File is either invalid or unicode, which is currently not supported.\n", errout.str());
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


    void define_part_of_func() {
        const char filedata[] = "#define A g(\n"
                                "void f() {\n"
                                "  A );\n"
                                "  }\n";

        // Preprocess => actual result..
        std::map<std::string, std::string> actual;
        preprocess(filedata, actual);

        // Compare results..
        ASSERT_EQUALS(1, static_cast<unsigned int>(actual.size()));
        ASSERT_EQUALS("\nvoid f ( ) {\n$g $( ) ;\n}", actual[""]);
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
        std::map<std::string, std::string> actual;
        preprocess(filedata, actual);

        // Compare results..
        ASSERT_EQUALS(2, static_cast<unsigned int>(actual.size()));
        ASSERT_EQUALS("\n\n\n\n\n$20", actual[""]);
        ASSERT_EQUALS("\n\n\n\n\n$10", actual["A"]);
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
        std::map<std::string, std::string> actual;
        preprocess(filedata, actual);

        // Compare results..
        ASSERT_EQUALS(1, static_cast<unsigned int>(actual.size()));
        ASSERT_EQUALS("", actual[""]);
        ASSERT_EQUALS("[file.c:6]: (error) failed to expand 'BC', Wrong number of parameters for macro 'BC'.\n", errout.str());
    }

    void newline_in_macro() {
        const char filedata[] = "#define ABC(str) printf( str )\n"
                                "void f()\n"
                                "{\n"
                                "  ABC(\"\\n\");\n"
                                "}\n";

        // Preprocess => actual result..
        std::map<std::string, std::string> actual;
        preprocess(filedata, actual);

        // Compare results..
        ASSERT_EQUALS(1, static_cast<unsigned int>(actual.size()));
        ASSERT_EQUALS("\nvoid f ( )\n{\n$printf $( \"\\n\" $) ;\n}", actual[""]);
        ASSERT_EQUALS("", errout.str());
    }

    void ifdef_ifdefined() {
        const char filedata[] = "#ifdef ABC\n"
                                "A\n"
                                "#endif\t\n"
                                "#if defined ABC\n"
                                "A\n"
                                "#endif\n";

        // Preprocess => actual result..
        std::map<std::string, std::string> actual;
        preprocess(filedata, actual);

        // Compare results..
        ASSERT_EQUALS("", actual[""]);
        ASSERT_EQUALS("\nA\n\n\nA", actual["ABC"]);
        ASSERT_EQUALS(2, static_cast<unsigned int>(actual.size()));
    }

    void define_if1() {
        {
            const char filedata[] = "#define A 0\n"
                                    "#if A\n"
                                    "FOO\n"
                                    "#endif";
            ASSERT_EQUALS("", preprocessor0.getcode(filedata,"",""));
        }
        {
            const char filedata[] = "#define A 1\n"
                                    "#if A==1\n"
                                    "FOO\n"
                                    "#endif";
            ASSERT_EQUALS("\n\nFOO", preprocessor0.getcode(filedata,"",""));
        }
    }

    void define_if2() {
        const char filedata[] = "#define A 22\n"
                                "#define B A\n"
                                "#if (B==A) || (B==C)\n"
                                "FOO\n"
                                "#endif";
        ASSERT_EQUALS("\n\n\nFOO", preprocessor0.getcode(filedata,"",""));
    }

    void define_if3() {
        const char filedata[] = "#define A 0\n"
                                "#if (A==0)\n"
                                "FOO\n"
                                "#endif";
        ASSERT_EQUALS("\n\nFOO", preprocessor0.getcode(filedata,"",""));
    }

    void define_if4() {
        const char filedata[] = "#define X +123\n"
                                "#if X==123\n"
                                "FOO\n"
                                "#endif";
        ASSERT_EQUALS("\n\nFOO", preprocessor0.getcode(filedata,"",""));
    }

    void define_if5() { // #4516 - #define B (A & 0x00f0)
        {
            const char filedata[] = "#define A 0x0010\n"
                                    "#define B (A & 0x00f0)\n"
                                    "#if B==0x0010\n"
                                    "FOO\n"
                                    "#endif";
            ASSERT_EQUALS("\n\n\nFOO", preprocessor0.getcode(filedata,"",""));
        }
        {
            const char filedata[] = "#define A 0x00f0\n"
                                    "#define B (16)\n"
                                    "#define C (B & A)\n"
                                    "#if C==0x0010\n"
                                    "FOO\n"
                                    "#endif";
            ASSERT_EQUALS("\n\n\n\nFOO", preprocessor0.getcode(filedata,"",""));
        }
        {
            const char filedata[] = "#define A (1+A)\n" // don't hang for recursive macros
                                    "#if A==1\n"
                                    "FOO\n"
                                    "#endif";
            ASSERT_EQUALS("\n\nFOO", preprocessor0.getcode(filedata,"",""));
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
        const std::string actualA0 = preprocessor0.getcode(filedata, "A=0", "test.c");
        ASSERT_EQUALS(true,  actualA0.find("123") != std::string::npos);
        ASSERT_EQUALS(false, actualA0.find("456") != std::string::npos);
        const std::string actualA1 = preprocessor0.getcode(filedata, "A=1", "test.c");
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
            std::map<std::string, std::string> actual;
            preprocess(filedata, actual);

            // Compare results..
            ASSERT_EQUALS(1, (int)actual.size());
            ASSERT_EQUALS("\n\n\n\nB", actual[""]);
        }

        {
            const char filedata[] = "#define A 1\n"
                                    "#ifdef A\n"
                                    "A\n"
                                    "#endif\n";

            // Preprocess => actual result..
            std::map<std::string, std::string> actual;
            preprocess(filedata, actual);

            // Compare results..
            ASSERT_EQUALS(1, (int)actual.size());
            ASSERT_EQUALS("\n\n$1", actual[""]);
        }

        {
            const char filedata[] = "#define A 1\n"
                                    "#if A==1\n"
                                    "A\n"
                                    "#endif\n";

            // Preprocess => actual result..
            std::map<std::string, std::string> actual;
            preprocess(filedata, actual);

            // Compare results..
            ASSERT_EQUALS(1, (int)actual.size());
            ASSERT_EQUALS("\n\n$1", actual[""]);
        }

        {
            const char filedata[] = "#define A 1\n"
                                    "#if A>0\n"
                                    "A\n"
                                    "#endif\n";

            // Preprocess => actual result..
            std::map<std::string, std::string> actual;
            preprocess(filedata, actual);

            // Compare results..
            ASSERT_EQUALS(1, (int)actual.size());
            ASSERT_EQUALS("\n\n$1", actual[""]);
        }

        {
            const char filedata[] = "#define A 1\n"
                                    "#if 0\n"
                                    "#undef A\n"
                                    "#endif\n"
                                    "A\n";

            // Preprocess => actual result..
            std::map<std::string, std::string> actual;
            preprocess(filedata, actual);

            // Compare results..
            ASSERT_EQUALS(1, (int)actual.size());
            ASSERT_EQUALS("\n\n\n\n$1", actual[""]);
        }
    }

    void define_ifndef1() {
        const char filedata[] = "#define A(x) (x)\n"
                                "#ifndef A\n"
                                ";\n"
                                "#endif\n";

        // Preprocess => actual result..
        std::map<std::string, std::string> actual;
        preprocess(filedata, actual);

        // Compare results..
        ASSERT_EQUALS(1U, actual.size());
        ASSERT_EQUALS("", actual[""]);
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
        ASSERT_EQUALS("\n\n\n\n\n\n$int me ;", preprocessor0.getcode(filedata, "", "a.cpp"));
        ASSERT_EQUALS("\n\n\n\n\n\n$char me ;", preprocessor0.getcode(filedata, "A", "a.cpp"));
    }

    void ifndef_define() {
        const char filedata[] = "#ifndef A\n"
                                "#define A(x) x\n"
                                "#endif\n"
                                "A(123);";

        // Preprocess => actual result..
        std::map<std::string, std::string> actual;
        preprocess(filedata, actual);

        ASSERT_EQUALS(1U, actual.size());
        ASSERT_EQUALS("\n\n\n123 ;", actual[""]);
    }

    void undef_ifdef() {
        const char filedata[] = "#undef A\n"
                                "#ifdef A\n"
                                "123\n"
                                "#endif\n";

        // Preprocess => actual result..
        ASSERT_EQUALS("", preprocessor0.getcode(filedata, "", "a.cpp"));
        ASSERT_EQUALS("", preprocessor0.getcode(filedata, "A", "a.cpp"));
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
        std::map<std::string, std::string> actual;
        preprocess(filedata, actual);

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
        std::map<std::string, std::string> actual;
        preprocess(filedata, actual);

        // Compare results..
        ASSERT_EQUALS("char a [ ] = \"#endfile\" ;\nchar b [ ] = \"#endfile\" ;", actual[""]);
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
        std::map<std::string, std::string> actual;
        preprocess(filedata, actual);

        // B will always be defined if A is defined; the following test
        // cases should be fixed whenever this other bug is fixed
        ASSERT_EQUALS(2U, actual.size());

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

    void invalid_define_1() {
        std::map<std::string, std::string> actual;
        preprocess("#define =\n", actual); // don't hang
    }

    void invalid_define_2() {  // #4036
        std::map<std::string, std::string> actual;
        preprocess("#define () {(int f(x) }\n", actual); // don't hang
    }

    void inline_suppression_for_missing_include() {
        Preprocessor::missingIncludeFlag = false;
        Settings settings;
        settings.inlineSuppressions = true;
        settings.addEnabled("all");
        Preprocessor preprocessor(settings, this);

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
        std::string actual = preprocessor0.getcode(src, "X=1", "test.c");

        ASSERT_EQUALS("\nFred & Wilma", actual);
    }

    void predefine2() {
        const std::string src("#if defined(X) && Y\n"
                              "Fred & Wilma\n"
                              "#endif\n");
        {
            std::string actual = preprocessor0.getcode(src, "X=1", "test.c");
            ASSERT_EQUALS("", actual);
        }

        {
            std::string actual = preprocessor0.getcode(src, "X=1;Y=2", "test.c");
            ASSERT_EQUALS("\nFred & Wilma", actual);
        }
    }

    void predefine3() {
        // #2871 - define in source is not used if -D is used
        const char code[] = "#define X 1\n"
                            "#define Y X\n"
                            "#if (X == Y)\n"
                            "Fred & Wilma\n"
                            "#endif\n";
        const std::string actual = preprocessor0.getcode(code, "TEST", "test.c");
        ASSERT_EQUALS("\n\n\nFred & Wilma", actual);
    }

    void predefine4() {
        // #3577
        const char code[] = "char buf[X];\n";
        const std::string actual = preprocessor0.getcode(code, "X=123", "test.c");
        ASSERT_EQUALS("char buf [ $123 ] ;", actual);
    }

    void predefine5() {  // #3737, #5119 - automatically define __cplusplus
        // #3737...
        const char code[] = "#ifdef __cplusplus\n123\n#endif";
        ASSERT_EQUALS("",      preprocessor0.getcode(code, "X=123", "test.c"));
        ASSERT_EQUALS("\n123", preprocessor0.getcode(code, "X=123", "test.cpp"));
    }

    void invalidElIf() {
        // #2942 - segfault
        const char code[] = "#elif (){\n";
        const std::string actual = preprocessor0.getcode(code, "TEST", "test.c");
        ASSERT_EQUALS("", actual);
    }

    void getConfigs1() {
        const char filedata[] = "#ifdef  WIN32 \n"
                                "    abcdef\n"
                                "#else  \n"
                                "    qwerty\n"
                                "#endif  \n";

        ASSERT_EQUALS("\nWIN32\n", getConfigsStr(filedata));
    }

    void getConfigs2() {
        const char filedata[] = "# ifndef WIN32\n"
                                "    \" # ifdef WIN32\" // a comment\n"
                                "   #   else  \n"
                                "    qwerty\n"
                                "  # endif  \n";
        ASSERT_EQUALS("\nWIN32\n", getConfigsStr(filedata));
    }

    void getConfigs3() {
        const char filedata[] = "#ifdef ABC\n"
                                "a\n"
                                "#ifdef DEF\n"
                                "b\n"
                                "#endif\n"
                                "c\n"
                                "#endif\n";

        ASSERT_EQUALS("\nABC\nABC;DEF\n", getConfigsStr(filedata));
    }

    void getConfigs4() {
        const char filedata[] = "#ifdef ABC\n"
                                "A\n"
                                "#endif\t\n"
                                "#ifdef ABC\n"
                                "A\n"
                                "#endif\n";
        ASSERT_EQUALS("\nABC\n", getConfigsStr(filedata));
    }

    void getConfigs5() {
        const char filedata[] = "#ifdef ABC\n"
                                "A\n"
                                "#else\n"
                                "B\n"
                                "#ifdef DEF\n"
                                "C\n"
                                "#endif\n"
                                "#endif\n";
        ASSERT_EQUALS("\nABC\nDEF\n", getConfigsStr(filedata));
    }

    void getConfigs7() {
        const char filedata[] = "#ifdef ABC\n"
                                "A\n"
                                "#ifdef ABC\n"
                                "B\n"
                                "#endif\n"
                                "#endif\n";
        ASSERT_EQUALS("\nABC\n", getConfigsStr(filedata));
    }

    void getConfigs7a() {
        const char filedata[] = "#ifndef ABC\n"
                                "A\n"
                                "#ifndef ABC\n"
                                "B\n"
                                "#endif\n"
                                "#endif\n";
        ASSERT_EQUALS("\n", getConfigsStr(filedata));
    }

    void getConfigs7b() {
        const char filedata[] = "#ifndef ABC\n"
                                "A\n"
                                "#ifdef ABC\n"
                                "B\n"
                                "#endif\n"
                                "#endif\n";
        ASSERT_EQUALS("\nABC\n", getConfigsStr(filedata));
    }

    void getConfigs7c() {
        const char filedata[] = "#ifdef ABC\n"
                                "A\n"
                                "#ifndef ABC\n"
                                "B\n"
                                "#endif\n"
                                "#endif\n";
        ASSERT_EQUALS("\nABC\n", getConfigsStr(filedata));
    }

    void getConfigs7d() {
        const char filedata[] = "#if defined(ABC)\n"
                                "A\n"
                                "#if defined(ABC)\n"
                                "B\n"
                                "#endif\n"
                                "#endif\n";
        ASSERT_EQUALS("\nABC\n", getConfigsStr(filedata));
    }

    void getConfigs7e() {
        const char filedata[] = "#ifdef ABC\n"
                                "#file \"test.h\"\n"
                                "#ifndef test_h\n"
                                "#define test_h\n"
                                "#ifdef ABC\n"
                                "#endif\n"
                                "#endif\n"
                                "#endfile\n"
                                "#endif\n";
        ASSERT_EQUALS("\nABC\n", getConfigsStr(filedata));
    }

    void getConfigs8() {
        const char filedata[] = "#if A == 1\n"
                                "1\n"
                                "#endif\n";
        ASSERT_EQUALS("\nA=1\n", getConfigsStr(filedata));
    }

    void getConfigs10() { // Ticket #5139
        const char filedata[] = "#define foo a.foo\n"
                                "#define bar foo\n"
                                "#define baz bar+0\n"
                                "#if 0\n"
                                "#endif";
        ASSERT_EQUALS("\n", getConfigsStr(filedata));
    }

    void getConfigsError() {
        const char filedata1[] = "#ifndef X\n"
                                 "#error \"!X\"\n"
                                 "#endif\n";
        ASSERT_EQUALS("\nX\n", getConfigsStr(filedata1));

        const char filedata2[] = "#ifdef X\n"
                                 "#ifndef Y\n"
                                 "#error \"!Y\"\n"
                                 "#endif\n"
                                 "#endif\n";
        ASSERT_EQUALS("\nX\nX;Y\n", getConfigsStr(filedata2));
    }

    void getConfigsD1() {
        const char filedata[] = "#ifdef X\n"
                                "#else\n"
                                "#ifdef Y\n"
                                "#endif\n"
                                "#endif\n";
        ASSERT_EQUALS("\n", getConfigsStr(filedata, "-DX"));
        ASSERT_EQUALS("\nX\nY\n", getConfigsStr(filedata));
    }

    void getConfigsU1() {
        const char filedata[] = "#ifdef X\n"
                                "#endif\n";
        ASSERT_EQUALS("\n", getConfigsStr(filedata, "-UX"));
        ASSERT_EQUALS("\nX\n", getConfigsStr(filedata));
    }

    void getConfigsU2() {
        const char filedata[] = "#ifndef X\n"
                                "#endif\n";
        ASSERT_EQUALS("\n", getConfigsStr(filedata, "-UX"));
        ASSERT_EQUALS("\n", getConfigsStr(filedata));  // no #else
    }

    void getConfigsU3() {
        const char filedata[] = "#ifndef X\n"
                                "Fred & Wilma\n"
                                "#else\n"
                                "Barney & Betty\n"
                                "#endif\n";
        ASSERT_EQUALS("\n", getConfigsStr(filedata, "-UX"));
        ASSERT_EQUALS("\nX\n", getConfigsStr(filedata));
    }

    void getConfigsU4() {
        const char filedata[] = "#if defined(X) || defined(Y) || defined(Z)\n"
                                "#else\n"
                                "#endif\n";
        ASSERT_EQUALS("\nY;Z\n", getConfigsStr(filedata, "-UX"));
        ASSERT_EQUALS("\nX;Y;Z\n", getConfigsStr(filedata));
    }

    void getConfigsU5() {
        const char filedata[] = "#if X==1\n"
                                "#endif\n";
        ASSERT_EQUALS("\n", getConfigsStr(filedata, "-UX"));
        ASSERT_EQUALS("\nX=1\n", getConfigsStr(filedata));
    }

    void getConfigsU6() {
        const char filedata[] = "#if X==0\n"
                                "#endif\n";
        ASSERT_EQUALS("\nX=0\n", getConfigsStr(filedata, "-UX"));
        ASSERT_EQUALS("\nX=0\n", getConfigsStr(filedata));
    }

    void getConfigsU7() {
        const char code[] = "#ifndef Y\n"
                            "#else\n"
                            "#endif\n";
        ASSERT_EQUALS("\nY\n", getConfigsStr(code, "-DX"));
    }


    void validateCfg() {
        Preprocessor preprocessor(settings0, this);

        std::list<simplecpp::MacroUsage> macroUsageList;
        std::vector<std::string> files(1, "test.c");
        simplecpp::MacroUsage macroUsage(files);
        macroUsage.useLocation.fileIndex = 0;
        macroUsage.useLocation.line = 1;
        macroUsage.macroName = "X";
        macroUsageList.push_back(macroUsage);

        ASSERT_EQUALS(true, preprocessor.validateCfg("", macroUsageList));
        ASSERT_EQUALS(false, preprocessor.validateCfg("X",macroUsageList));
        ASSERT_EQUALS(false, preprocessor.validateCfg("A=42;X", macroUsageList));
        ASSERT_EQUALS(true, preprocessor.validateCfg("X=1", macroUsageList));
        ASSERT_EQUALS(true, preprocessor.validateCfg("Y", macroUsageList));
    }

    void if_sizeof() { // #4071
        static const char* code = "#if sizeof(unsigned short) == 2\n"
                                  "Fred & Wilma\n"
                                  "#elif sizeof(unsigned short) == 4\n"
                                  "Fred & Wilma\n"
                                  "#else\n"
                                  "#endif";

        std::map<std::string, std::string> actual;
        preprocess(code, actual);
        ASSERT_EQUALS("\nFred & Wilma", actual[""]);
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
        std::map<std::string, std::string> actual;
        preprocess(filedata, actual);
    }

    void garbage() {
        const char filedata[] = "V\n"
                                "#define X b   #endif #line 0 \"x\"  ;\n"
                                "#if ! defined ( Y )    #endif";

        // Preprocess => don't crash..
        std::map<std::string, std::string> actual;
        preprocess(filedata, actual);
    }

    void wrongPathOnErrorDirective() {
        errout.str("");
        Settings settings;
        settings.userDefines = "foo";
        Preprocessor preprocessor(settings, this);
        const std::string code("#error hello world!\n");
        preprocessor.getcode(code, "X", "./././test.c");
        ASSERT_EQUALS("[test.c:1]: (error) #error hello world!\n", errout.str());
    }

    void testDirectiveIncludeTypes() {
        const char filedata[] = "#define macro some definition\n"
                                "#undef macro\n"
                                "#ifdef macro\n"
                                "#elif some (complex) condition\n"
                                "#else\n"
                                "#endif\n"
                                "#if some other condition\n"
                                "#pragma some proprietary content\n"
                                "#\n" /* may appear in old C code */
                                "#ident some text\n" /* may appear in old C code */
                                "#unknownmacro some unpredictable text\n"
                                "#warning some warning message\n"
                                "#error some error message\n";
        const char dumpdata[] = "  <directivelist>\n"

                                "    <directive file=\"test.c\" linenr=\"1\" str=\"#define macro some definition\"/>\n"
                                "    <directive file=\"test.c\" linenr=\"2\" str=\"#undef macro\"/>\n"
                                "    <directive file=\"test.c\" linenr=\"3\" str=\"#ifdef macro\"/>\n"
                                "    <directive file=\"test.c\" linenr=\"4\" str=\"#elif some (complex) condition\"/>\n"
                                "    <directive file=\"test.c\" linenr=\"5\" str=\"#else\"/>\n"
                                "    <directive file=\"test.c\" linenr=\"6\" str=\"#endif\"/>\n"
                                "    <directive file=\"test.c\" linenr=\"7\" str=\"#if some other condition\"/>\n"
                                "    <directive file=\"test.c\" linenr=\"8\" str=\"#pragma some proprietary content\"/>\n"
                                "    <directive file=\"test.c\" linenr=\"9\" str=\"#\"/>\n"
                                "    <directive file=\"test.c\" linenr=\"10\" str=\"#ident some text\"/>\n"
                                "    <directive file=\"test.c\" linenr=\"11\" str=\"#unknownmacro some unpredictable text\"/>\n"
                                "    <directive file=\"test.c\" linenr=\"12\" str=\"#warning some warning message\"/>\n"
                                "    <directive file=\"test.c\" linenr=\"13\" str=\"#error some error message\"/>\n"
                                "  </directivelist>\n";

        std::ostringstream ostr;
        Preprocessor preprocessor(settings0, this);
        preprocessor.getcode(filedata, "", "test.c");
        preprocessor.dump(ostr);
        ASSERT_EQUALS(dumpdata, ostr.str());
    }

    void testDirectiveIncludeLocations() {
        const char filedata[] = "#define macro1 val\n"
                                "#file \"inc1.h\"\n"
                                "#define macro2 val\n"
                                "#file \"inc2.h\"\n"
                                "#define macro3 val\n"
                                "#endfile\n"
                                "#define macro4 val\n"
                                "#endfile\n"
                                "#define macro5 val\n";
        const char dumpdata[] = "  <directivelist>\n"
                                "    <directive file=\"test.c\" linenr=\"1\" str=\"#define macro1 val\"/>\n"
                                "    <directive file=\"test.c\" linenr=\"2\" str=\"#include &quot;inc1.h&quot;\"/>\n"
                                "    <directive file=\"inc1.h\" linenr=\"1\" str=\"#define macro2 val\"/>\n"
                                "    <directive file=\"inc1.h\" linenr=\"2\" str=\"#include &quot;inc2.h&quot;\"/>\n"
                                "    <directive file=\"inc2.h\" linenr=\"1\" str=\"#define macro3 val\"/>\n"
                                "    <directive file=\"inc1.h\" linenr=\"3\" str=\"#define macro4 val\"/>\n"
                                "    <directive file=\"test.c\" linenr=\"3\" str=\"#define macro5 val\"/>\n"
                                "  </directivelist>\n";

        std::ostringstream ostr;
        Preprocessor preprocessor(settings0, this);
        preprocessor.getcode(filedata, "", "test.c");
        preprocessor.dump(ostr);
        ASSERT_EQUALS(dumpdata, ostr.str());
    }

    void testDirectiveIncludeComments() {
        const char filedata[] = "#ifdef macro2 /* this will be removed */\n"
                                "#else /* this will be removed too */\n"
                                "#endif /* this will also be removed */\n";
        const char dumpdata[] = "  <directivelist>\n"
                                "    <directive file=\"test.c\" linenr=\"1\" str=\"#ifdef macro2\"/>\n"
                                "    <directive file=\"test.c\" linenr=\"2\" str=\"#else\"/>\n"
                                "    <directive file=\"test.c\" linenr=\"3\" str=\"#endif\"/>\n"
                                "  </directivelist>\n";

        std::ostringstream ostr;
        Preprocessor preprocessor(settings0, this);
        preprocessor.getcode(filedata, "", "test.c");
        preprocessor.dump(ostr);
        ASSERT_EQUALS(dumpdata, ostr.str());
    }

    void testSameLine() { // Ticket #7912
        const char code[] = "#line 1 \"bench/btl/libs/BLAS/blas_interface_impl.hh\" \n"
                            "template < > class blas_interface < float > : public c_interface_base < float > \n"
                            "{ } ;\n"
                            "#line 1 \"bench/btl/libs/BLAS/blas_interface_impl.hh\" \n"
                            "template < > class blas_interface < double > : public c_interface_base < double > \n"
                            "{ } ;";
        const char exp[]  = "template < > class blas_interface < float > : public c_interface_base < float >\n"
                            "{ } ; template < > class blas_interface < double > : public c_interface_base < double > { } ;";
        Preprocessor preprocessor(settings0, this);
        ASSERT_EQUALS(exp, preprocessor.getcode(code, "", "test.cpp"));
    }

};

REGISTER_TEST(TestPreprocessor)
