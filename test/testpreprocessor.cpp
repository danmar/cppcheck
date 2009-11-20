/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2009 Daniel Marjamäki and Cppcheck team.
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
#include <map>
#include <string>
#include <sstream>
#include <stdexcept>

extern std::ostringstream errout;

class TestPreprocessor : public TestFixture
{
public:
    TestPreprocessor() : TestFixture("TestPreprocessor")
    { }

    class OurPreprocessor : public Preprocessor
    {
    public:
        static std::string replaceIfDefined(const std::string &str)
        {
            return Preprocessor::replaceIfDefined(str);
        }

        static std::string expandMacros(std::string code, ErrorLogger *errorLogger = 0)
        {
            return Preprocessor::expandMacros(code, "file.cpp", errorLogger);
        }

        static int getHeaderFileName(std::string &str)
        {
            return Preprocessor::getHeaderFileName(str);
        }
    };

private:

    void run()
    {
        // Just read the code into a string. Perform simple cleanup of the code
        TEST_CASE(readCode);

        // The bug that started the whole work with the new preprocessor
        TEST_CASE(Bug2190219);

        TEST_CASE(test1);
        TEST_CASE(test2);
        TEST_CASE(test3);
        TEST_CASE(test4);
        TEST_CASE(test5);
        TEST_CASE(test6);
        TEST_CASE(test7);

        // #error => don't extract any code
        TEST_CASE(error1);

        // Handling include guards (don't create extra configuration for it)
        TEST_CASE(includeguard);

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
        TEST_CASE(macroInMacro);
        TEST_CASE(macro_mismatch);
        TEST_CASE(macro_linenumbers);
        TEST_CASE(macro_nopar);
        TEST_CASE(string1);
        TEST_CASE(string2);
        TEST_CASE(preprocessor_undef);
        TEST_CASE(defdef);  // Defined multiple times
        TEST_CASE(preprocessor_doublesharp);
        TEST_CASE(preprocessor_include_in_str);
        TEST_CASE(fmt1);
        TEST_CASE(fmt2);
        TEST_CASE(fmt3);
        TEST_CASE(multi_character_character);

        TEST_CASE(stringify);
        TEST_CASE(stringify2);
        TEST_CASE(stringify3);
        TEST_CASE(stringify4);
        TEST_CASE(stringify5);
        TEST_CASE(ifdefwithfile);
        TEST_CASE(pragma);
        TEST_CASE(pragma_asm);
        TEST_CASE(endifsemicolon);
        TEST_CASE(missing_doublequote);
        TEST_CASE(handle_error);

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
        TEST_CASE(define_ifdef);
        TEST_CASE(endfile);
    }


    void readCode()
    {
        const char code[] = " \t a //\n"
                            "  #aa\t /* remove this */\tb  \r\n";
        Preprocessor p;
        std::istringstream istr(code);
        std::string codestr(p.read(istr));
        ASSERT_EQUALS("a \n#aa b \n", codestr);
    }


    void Bug2190219()
    {
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
        Preprocessor preprocessor;
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS(expected[""], actual[""]);
        ASSERT_EQUALS(expected["__cplusplus"], actual["__cplusplus"]);
        ASSERT_EQUALS(2, static_cast<unsigned int>(actual.size()));
    }


    void test1()
    {
        const char filedata[] = "#ifdef  WIN32 \n"
                                "    abcdef\n"
                                "#else  \n"
                                "    qwerty\n"
                                "#endif  \n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Preprocessor preprocessor;
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS("\n\n\nqwerty\n\n", actual[""]);
        ASSERT_EQUALS("\nabcdef\n\n\n\n", actual["WIN32"]);
        ASSERT_EQUALS(2, static_cast<unsigned int>(actual.size()));
    }

    void test2()
    {
        const char filedata[] = "# ifndef WIN32\n"
                                "    \" # ifdef WIN32\" // a comment\n"
                                "   #   else  \n"
                                "    qwerty\n"
                                "  # endif  \n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Preprocessor preprocessor;
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS("\n\" #ifdef WIN32\"\n\n\n\n", actual[""]);
        ASSERT_EQUALS("\n\n\nqwerty\n\n", actual["WIN32"]);
        ASSERT_EQUALS(2, static_cast<unsigned int>(actual.size()));
    }

    void test3()
    {
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
        Preprocessor preprocessor;
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS("\n\n\n\n\n\n\n", actual[""]);
        ASSERT_EQUALS("\na\n\n\n\nc\n\n", actual["ABC"]);
        ASSERT_EQUALS("\na\n\nb\n\nc\n\n", actual["ABC;DEF"]);
        ASSERT_EQUALS(3, static_cast<unsigned int>(actual.size()));
    }

    void test4()
    {
        const char filedata[] = "#ifdef ABC\n"
                                "A\n"
                                "#endif\t\n"
                                "#ifdef ABC\n"
                                "A\n"
                                "#endif\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Preprocessor preprocessor;
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS("\n\n\n\n\n\n", actual[""]);
        ASSERT_EQUALS("\nA\n\n\nA\n\n", actual["ABC"]);
        ASSERT_EQUALS(2, static_cast<unsigned int>(actual.size()));
    }

    void test5()
    {
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
        Preprocessor preprocessor;
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS("\n\n\nB\n\n\n\n\n", actual[""]);
        ASSERT_EQUALS("\nA\n\n\n\n\n\n\n", actual["ABC"]);
        ASSERT_EQUALS("\n\n\nB\n\nC\n\n\n", actual["DEF"]);
        ASSERT_EQUALS(3, static_cast<unsigned int>(actual.size()));
    }

    void test6()
    {
        const char filedata[] = "#if(A)\n"
                                "#if ( A ) \n"
                                "#if A\n"
                                "#if defined((A))\n"
                                "#elif defined (A)\n";

        std::istringstream istr(filedata);
        const std::string actual(Preprocessor::read(istr));

        // Compare results..
        ASSERT_EQUALS("#if A\n#if A\n#if A\n#if defined(A)\n#elif defined(A)\n", actual);
    }

    void test7()
    {
        const char filedata[] = "#ifdef ABC\n"
                                "A\n"
                                "#ifdef ABC\n"
                                "B\n"
                                "#endif\n"
                                "#endif\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Preprocessor preprocessor;
        errout.str("");
        preprocessor.preprocess(istr, actual, "file.c");

        // Make sure an error message is written..
        ASSERT_EQUALS("", errout.str());    // no change?
        TODO_ASSERT_EQUALS("[test.cpp:3]: this preprocessor condition is always true", errout.str());

        // Compare results..
        ASSERT_EQUALS("\n\n\n\n\n\n", actual[""]);
        ASSERT_EQUALS("\nA\n\nB\n\n\n", actual["ABC"]);
        ASSERT_EQUALS(2, static_cast<unsigned int>(actual.size()));
    }


    void error1()
    {
        const char filedata[] = "#ifdef A\n"
                                ";\n"
                                "#else\n"
                                "#error abcd\n"
                                "#endif\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Preprocessor preprocessor;
        errout.str("");
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS(2, static_cast<unsigned int>(actual.size()));
        ASSERT_EQUALS("", actual[""]);
        ASSERT_EQUALS("\n;\n\n\n\n", actual["A"]);

    }


    void includeguard()
    {
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
        Preprocessor preprocessor;
        preprocessor.preprocess(istr, actual, "file.c");

        // Expected configurations: "" and "ABC"
        ASSERT_EQUALS(2, static_cast<unsigned int>(actual.size()));
    }

    void ifdefwithfile()
    {
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
        Preprocessor preprocessor;
        preprocessor.preprocess(istr, actual, "file.c");

        Tokenizer tok;
        std::istringstream codeStream(actual[""]);
        tok.tokenize(codeStream, "main.cpp");

        ASSERT_EQUALS("\n\n##file 0\n1:\n2:\n3:\n4: int main ( ) { }\n", tok.tokens()->stringifyList());

        // Expected configurations: "" and "ABC"
        ASSERT_EQUALS(2, static_cast<unsigned int>(actual.size()));
        ASSERT_EQUALS("\n#file \"abc.h\"\n\n\n\n\n\n\n\n\n#endfile\n\nint main() {}\n", actual[""]);
        ASSERT_EQUALS("\n#file \"abc.h\"\nclass A{};\n\n\n\n\n\n\n\n#endfile\n\nint main() {}\n", actual["ABC"]);
    }

    void newlines()
    {
        const char filedata[] = "\r\r\n\n";

        // Preprocess
        std::istringstream istr(filedata);
        Preprocessor preprocessor;
        ASSERT_EQUALS("\n\n\n", preprocessor.read(istr));
    }



    void comments1()
    {
        {
            const char filedata[] = "/*\n"
                                    "#ifdef WIN32\n"
                                    "#endif\n"
                                    "*/\n";

            // Preprocess => actual result..
            std::istringstream istr(filedata);
            std::map<std::string, std::string> actual;
            Preprocessor preprocessor;
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
            Preprocessor preprocessor;
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
            Preprocessor preprocessor;
            preprocessor.preprocess(istr, actual, "file.c");

            // Compare results..
            ASSERT_EQUALS("void f()\n{\n*p = a / *b / *c;\n}\n", actual[""]);
            ASSERT_EQUALS(1, static_cast<unsigned int>(actual.size()));
        }
    }



    void if0()
    {
        const char filedata[] = " # if /* comment */  0 // comment\n"
                                "#ifdef WIN32\n"
                                "#endif\n"
                                "#endif\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Preprocessor preprocessor;
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS("\n\n\n\n", actual[""]);
        ASSERT_EQUALS(1, static_cast<unsigned int>(actual.size()));
    }

    void if1()
    {
        const char filedata[] = " # if /* comment */  1 // comment\n"
                                "ABC\n"
                                " # endif \n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Preprocessor preprocessor;
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS("\nABC\n\n", actual[""]);
        ASSERT_EQUALS(1, static_cast<unsigned int>(actual.size()));
    }


    void elif()
    {
        const char filedata[] = "#if DEF1\n"
                                "ABC\n"
                                "#elif DEF2\n"
                                "DEF\n"
                                "#endif\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Preprocessor preprocessor;
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS("\n\n\n\n\n", actual[""]);
        ASSERT_EQUALS("\nABC\n\n\n\n", actual["DEF1"]);
        ASSERT_EQUALS("\n\n\nDEF\n\n", actual["DEF2"]);
        ASSERT_EQUALS(3, static_cast<unsigned int>(actual.size()));
    }



    void match_cfg_def()
    {
        {
            std::map<std::string, std::string> cfg;
            cfg["ABC"] = "";

            ASSERT_EQUALS(false, Preprocessor::match_cfg_def(cfg, "defined(A)"));
            TODO_ASSERT_EQUALS(true, Preprocessor::match_cfg_def(cfg, "!defined(A)"));

            ASSERT_EQUALS(false, Preprocessor::match_cfg_def(cfg, "!defined(ABC)&&!defined(DEF)"));
            TODO_ASSERT_EQUALS(true, Preprocessor::match_cfg_def(cfg, "!defined(A)&&!defined(B)"));
        }

        {
            std::map<std::string, std::string> cfg;
            cfg["A"] = "1";
            cfg["B"] = "2";

            TODO_ASSERT_EQUALS(true, Preprocessor::match_cfg_def(cfg, "A==1"));
            TODO_ASSERT_EQUALS(true, Preprocessor::match_cfg_def(cfg, "A<2"));
            ASSERT_EQUALS(false, Preprocessor::match_cfg_def(cfg, "A==2"));
            ASSERT_EQUALS(false, Preprocessor::match_cfg_def(cfg, "A<1"));
            TODO_ASSERT_EQUALS(true, Preprocessor::match_cfg_def(cfg, "A>=1&&B<=A"));
        }
    }


    void if_cond1()
    {
        const char filedata[] = "#if LIBVER>100\n"
                                "    A\n"
                                "#else\n"
                                "    B\n"
                                "#endif\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Preprocessor preprocessor;
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS("\n\n\nB\n\n", actual[""]);
        ASSERT_EQUALS("\nA\n\n\n\n", actual["LIBVER>100"]);
        ASSERT_EQUALS(2, static_cast<unsigned int>(actual.size()));
    }

    void if_cond2()
    {
        const char filedata[] = "#ifdef A\n"
                                "a\n"
                                "#endif\n"
                                "#if defined(A) && defined(B)\n"
                                "ab\n"
                                "#endif\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Preprocessor preprocessor;
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS(3, static_cast<unsigned int>(actual.size()));
        ASSERT_EQUALS("\n\n\n\n\n\n", actual[""]);
        ASSERT_EQUALS("\na\n\n\n\n\n", actual["A"]);
        ASSERT_EQUALS("\na\n\n\nab\n\n", actual["A;B"]);
    }

    void if_cond3()
    {
        const char filedata[] = "#ifdef A\n"
                                "a\n"
                                "#if defined(B) && defined(C)\n"
                                "abc\n"
                                "#endif\n"
                                "#endif\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Preprocessor preprocessor;
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS(3, static_cast<unsigned int>(actual.size()));
        ASSERT_EQUALS("\n\n\n\n\n\n", actual[""]);
        ASSERT_EQUALS("\na\n\n\n\n\n", actual["A"]);
        ASSERT_EQUALS("\na\n\nabc\n\n\n", actual["A;B;C"]);
    }

    void if_cond4()
    {
        {
            const char filedata[] = "#define A\n"
                                    "#define B\n"
                                    "#if defined A || defined B\n"
                                    "ab\n"
                                    "#endif\n";

            // Preprocess => actual result..
            std::istringstream istr(filedata);
            std::map<std::string, std::string> actual;
            Preprocessor preprocessor;
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
            Preprocessor preprocessor;
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
            Preprocessor preprocessor;
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
            Preprocessor preprocessor;
            preprocessor.preprocess(istr, actual, "file.c");

            // Compare results..
            ASSERT_EQUALS(2, static_cast<unsigned int>(actual.size()));
            ASSERT_EQUALS("\n\n\n", actual[""]);
            ASSERT_EQUALS("\nfoo();\n\n", actual["A"]);
        }
    }

    void if_cond5()
    {
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
        Preprocessor preprocessor;
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS(2, static_cast<unsigned int>(actual.size()));
        ASSERT_EQUALS("\n\n\ncd\n\n\n\n", actual[""]);
        ASSERT_EQUALS("\nab\n\ncd\n\nef\n\n", actual["A;B"]);
    }

    void if_cond6()
    {
        const char filedata[] = "\n"
                                "#if defined(A) && defined(B))\n"
                                "#endif\n";

        errout.str("");

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        const Settings settings;
        Preprocessor preprocessor(&settings, this);
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS("[file.c:2]: (error) mismatching number of '(' and ')' in this line: defined(A)&&defined(B))\n", errout.str());
    }


    void multiline1()
    {
        const char filedata[] = "#define str \"abc\"     \\\n"
                                "            \"def\"       \n"
                                "abcdef = str;\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        Preprocessor preprocessor;
        ASSERT_EQUALS("#define str \"abc\" \"def\" \n\nabcdef = str;\n", preprocessor.read(istr));
    }

    void multiline2()
    {
        const char filedata[] = "#define sqr(aa) aa * \\\n"
                                "                aa\n"
                                "sqr(5);\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        Preprocessor preprocessor;
        ASSERT_EQUALS("#define sqr(aa) aa * aa\n\nsqr(5);\n", preprocessor.read(istr));
    }

    void multiline3()
    {
        const char filedata[] = "const char *str = \"abc\\\n"
                                "def\\\n"
                                "ghi\"\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        Preprocessor preprocessor;
        ASSERT_EQUALS("const char *str = \"abcdefghi\"\n\n\n", preprocessor.read(istr));
    }

    void multiline4()
    {
        errout.str("");
        const char filedata[] = "#define A int a = 4;\\ \n"
                                " int b = 5;\n"
                                "A\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Preprocessor preprocessor;
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS(1, static_cast<unsigned int>(actual.size()));
        ASSERT_EQUALS("\n\nint a = 4; int b = 5;\n", actual[""]);
        ASSERT_EQUALS("", errout.str());
    }

    void multiline5()
    {
        errout.str("");
        const char filedata[] = "#define ABC int a /*\n"
                                "*/= 4;\n"
                                "int main(){\n"
                                "ABC\n"
                                "}\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Preprocessor preprocessor;
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS(1, static_cast<unsigned int>(actual.size()));
        ASSERT_EQUALS("\n\nint main(){\nint a = 4;\n}\n", actual[""]);
        ASSERT_EQUALS("", errout.str());
    }

    void remove_asm()
    {
        std::string str1("\nasm(\n\n\n);");
        Preprocessor::removeAsm(str1);
        ASSERT_EQUALS("\nasm()\n\n\n;", str1);

        std::string str2("\nasm __volatile(\"\nlw iScale, 0x00(pScale)\n\", ());");
        Preprocessor::removeAsm(str2);
        ASSERT_EQUALS("\n\n\n;", str2);
    }

    void if_defined()
    {
        {
            const char filedata[] = "#if defined(AAA)\n"
                                    "#endif\n";
            ASSERT_EQUALS("#ifdef AAA\n#endif\n", OurPreprocessor::replaceIfDefined(filedata));
        }

        {
            ASSERT_EQUALS("#elif A\n", OurPreprocessor::replaceIfDefined("#elif defined(A)\n"));
        }
    }

    void if_not_defined()
    {
        const char filedata[] = "#if !defined(AAA)\n"
                                "#endif\n";
        ASSERT_EQUALS("#ifndef AAA\n#endif\n", OurPreprocessor::replaceIfDefined(filedata));
    }


    void macro_simple1()
    {
        {
            const char filedata[] = "#define AAA(aa) f(aa)\n"
                                    "AAA(5);\n";
            ASSERT_EQUALS("\nf(5);\n", OurPreprocessor::expandMacros(filedata));
        }

        {
            const char filedata[] = "#define AAA(aa) f(aa)\n"
                                    "AAA (5);\n";
            ASSERT_EQUALS("\nf(5);\n", OurPreprocessor::expandMacros(filedata));
        }
    }

    void macro_simple2()
    {
        const char filedata[] = "#define min(x,y) x<y?x:y\n"
                                "min(a(),b());\n";
        ASSERT_EQUALS("\na()<b()?a():b();\n", OurPreprocessor::expandMacros(filedata));
    }

    void macro_simple3()
    {
        const char filedata[] = "#define A 4\n"
                                "A AA\n";
        ASSERT_EQUALS("\n4 AA\n", OurPreprocessor::expandMacros(filedata));
    }

    void macro_simple4()
    {
        const char filedata[] = "#define TEMP_1 if( temp > 0 ) return 1;\n"
                                "TEMP_1\n";
        ASSERT_EQUALS("\nif( temp > 0 ) return 1;\n", OurPreprocessor::expandMacros(filedata));
    }

    void macro_simple5()
    {
        const char filedata[] = "#define ABC if( temp > 0 ) return 1;\n"
                                "\n"
                                "void foo()\n"
                                "{\n"
                                "    int temp = 0;\n"
                                "    ABC\n"
                                "}\n";
        ASSERT_EQUALS("\n\nvoid foo()\n{\n    int temp = 0;\n    if( temp > 0 ) return 1;\n}\n", OurPreprocessor::expandMacros(filedata));
    }

    void macro_simple6()
    {
        const char filedata[] = "#define ABC (a+b+c)\n"
                                "ABC";
        ASSERT_EQUALS("\n(a+b+c)", OurPreprocessor::expandMacros(filedata));
    }

    void macro_simple7()
    {
        const char filedata[] = "#define ABC(str) str\n"
                                "ABC(\"(\")";
        ASSERT_EQUALS("\n\"(\"", OurPreprocessor::expandMacros(filedata));
    }

    void macro_simple8()
    {
        const char filedata[] = "#define ABC 123\n"
                                "#define ABCD 1234\n"
                                "ABC ABCD";
        ASSERT_EQUALS("\n\n123 1234", OurPreprocessor::expandMacros(filedata));
    }

    void macro_simple9()
    {
        const char filedata[] = "#define ABC(a) f(a)\n"
                                "ABC( \"\\\"\" );\n"
                                "ABC( \"g\" );";
        ASSERT_EQUALS("\nf(\"\\\"\");\nf(\"g\");", OurPreprocessor::expandMacros(filedata));
    }

    void macro_simple10()
    {
        const char filedata[] = "#define ABC(t) t x\n"
                                "ABC(unsigned long);";
        ASSERT_EQUALS("\nunsigned long x;", OurPreprocessor::expandMacros(filedata));
    }

    void macroInMacro()
    {
        {
            const char filedata[] = "#define A(m) long n = m; n++;\n"
                                    "#define B(n) A(n)\n"
                                    "B(0)";
            ASSERT_EQUALS("\n\nlong n=0;n++;", OurPreprocessor::expandMacros(filedata));
        }

        {
            const char filedata[] = "#define A B\n"
                                    "#define B 3\n"
                                    "A";
            ASSERT_EQUALS("\n\n3", OurPreprocessor::expandMacros(filedata));
        }

        {
            const char filedata[] = "#define DBG(fmt, args...) printf(fmt, ## args)\n"
                                    "#define D(fmt, args...) DBG(fmt, ## args)\n"
                                    "DBG(\"hello\");";
            ASSERT_EQUALS("\n\nprintf(\"hello\");", OurPreprocessor::expandMacros(filedata));
        }

        {
            const char filedata[] = "#define DBG(fmt, args...) printf(fmt, ## args)\n"
                                    "#define D(fmt, args...) DBG(fmt, ## args)\n"
                                    "DBG(\"hello: %d\",3);";
            ASSERT_EQUALS("\n\nprintf(\"hello: %d\",3);", OurPreprocessor::expandMacros(filedata));
        }

        {
            const char filedata[] = "#define A 4\n"
                                    "#define B(a) a,A\n"
                                    "B(2);";
            ASSERT_EQUALS("\n\n2,4;", OurPreprocessor::expandMacros(filedata));
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
            ASSERT_EQUALS("\n\nfoo ((i)*2);\n", OurPreprocessor::expandMacros(filedata));
        }

        {
            const char filedata[] = "#define foo foo\n"
                                    "foo\n";
            ASSERT_EQUALS("\nfoo\n", OurPreprocessor::expandMacros(filedata));
        }
    }

    void macro_mismatch()
    {
        const char filedata[] = "#define AAA(aa,bb) f(aa)\n"
                                "AAA(5);\n";
        ASSERT_EQUALS("\nAAA(5);\n", OurPreprocessor::expandMacros(filedata));
    }

    void macro_linenumbers()
    {
        const char filedata[] = "#define AAA(a)\n"
                                "AAA(5\n"
                                "\n"
                                ")\n"
                                "int a;\n";
        ASSERT_EQUALS("\n"
                      "\n"
                      "\n"
                      "\n"
                      "int a;\n",
                      OurPreprocessor::expandMacros(filedata));
    }

    void macro_nopar()
    {
        const char filedata[] = "#define AAA( ) { NULL }\n"
                                "AAA()";
        ASSERT_EQUALS("\n{ NULL }", OurPreprocessor::expandMacros(filedata));
    }


    void string1()
    {
        const char filedata[] = "int main()"
                                "{"
                                "    const char *a = \"#define A\n\";"
                                "}";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Preprocessor preprocessor;
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS(1, static_cast<unsigned int>(actual.size()));
        ASSERT_EQUALS("int main(){ const char *a = \"#define A\n\";}\n", actual[""]);
    }

    void string2()
    {
        const char filedata[] = "#define AAA 123\n"
                                "str = \"AAA\"";

        // Compare results..
        ASSERT_EQUALS("\nstr = \"AAA\"", OurPreprocessor::expandMacros(filedata));
    }


    void preprocessor_undef()
    {
        {
            const char filedata[] = "#define AAA int a;\n"
                                    "#undef AAA\n"
                                    "#define AAA char b=0;\n"
                                    "AAA\n";

            // Compare results..
            ASSERT_EQUALS("\n\n\nchar b=0;\n", OurPreprocessor::expandMacros(filedata));
        }

        {
            // ticket #403
            const char filedata[] = "#define z p[2]\n"
                                    "#undef z\n"
                                    "int z;\n"
                                    "z = 0;\n";

            ASSERT_EQUALS("\n\nint z;\nz = 0;\n", OurPreprocessor::getcode(filedata, "", "", NULL));
        }
    }

    void defdef()
    {
        const char filedata[] = "#define AAA 123\n"
                                "#define AAA 456\n"
                                "#define AAA 789\n"
                                "AAA\n";

        // Compare results..
        ASSERT_EQUALS("\n\n\n789\n", OurPreprocessor::expandMacros(filedata));
    }

    void preprocessor_doublesharp()
    {
        const char filedata[] = "#define TEST(var,val) var = val\n"
                                "TEST(foo,20);\n";

        // Compare results..
        ASSERT_EQUALS("\nfoo=20;\n", OurPreprocessor::expandMacros(filedata));

        const char filedata2[] = "#define TEST(var,val) var##_##val = val\n"
                                 "TEST(foo,20);\n";

        // Compare results..
        ASSERT_EQUALS("\nfoo_20=20;\n", OurPreprocessor::expandMacros(filedata2));
    }



    void preprocessor_include_in_str()
    {
        const char filedata[] = "int main()\n"
                                "{\n"
                                "const char *a = \"#include <string>\n\";\n"
                                "return 0;\n"
                                "}\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Preprocessor preprocessor;
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS(1, static_cast<unsigned int>(actual.size()));
        ASSERT_EQUALS("int main()\n{\nconst char *a = \"#include <string>\n\";\nreturn 0;\n}\n", actual[""]);
    }




    void fmt1()
    {
        const char filedata[] = "#define DBG(fmt...) printf(fmt)\n"
                                "DBG(\"[0x%lx-0x%lx)\", pstart, pend);";

        // Preprocess..
        std::string actual = OurPreprocessor::expandMacros(filedata);

        ASSERT_EQUALS("\nprintf(\"[0x%lx-0x%lx)\",pstart,pend);", actual);
    }

    void fmt2()
    {
        const char filedata[] = "#define DBG(fmt, args...) printf(fmt, ## args)\n"
                                "DBG(\"hello\");";

        // Preprocess..
        std::string actual = OurPreprocessor::expandMacros(filedata);

        ASSERT_EQUALS("\nprintf(\"hello\");", actual);
    }

    void fmt3()
    {
        const char filedata[] = "#define FRED(...) { fred(__VA_ARGS__); }\n"
                                "FRED(123)";
        ASSERT_EQUALS("\n{ fred(123); }", OurPreprocessor::expandMacros(filedata));
    }



    void multi_character_character()
    {
        const char filedata[] = "#define FOO 'ABCD'\n"
                                "int main()\n"
                                "{\n"
                                "if( FOO == 0 );\n"
                                "return 0;\n"
                                "}\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Preprocessor preprocessor;
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS(1, static_cast<unsigned int>(actual.size()));
        ASSERT_EQUALS("\nint main()\n{\nif( 'ABCD' == 0 );\nreturn 0;\n}\n", actual[""]);
    }


    void stringify()
    {
        const char filedata[] = "#define STRINGIFY(x) #x\n"
                                "STRINGIFY(abc)";

        // expand macros..
        std::string actual = OurPreprocessor::expandMacros(filedata);

        ASSERT_EQUALS("\n\"abc\"", actual);
    }

    void stringify2()
    {
        const char filedata[] = "#define A(x) g(#x)\n"
                                "A(abc);";

        // expand macros..
        std::string actual = OurPreprocessor::expandMacros(filedata);

        ASSERT_EQUALS("\ng(\"abc\");", actual);
    }

    void stringify3()
    {
        const char filedata[] = "#define A(x) g(#x)\n"
                                "A( abc);";

        // expand macros..
        std::string actual = OurPreprocessor::expandMacros(filedata);

        ASSERT_EQUALS("\ng(\"abc\");", actual);
    }

    void stringify4()
    {
        const char filedata[] = "#define A(x) #x\n"
                                "1 A(\n"
                                "abc\n"
                                ") 2";

        // expand macros..
        std::string actual = OurPreprocessor::expandMacros(filedata);

        ASSERT_EQUALS("\n1 \n\n\"abc\" 2", actual);
    }

    void stringify5()
    {
        const char filedata[] = "#define A(x) a(#x,x)\n"
                                "A(foo(\"\\\"\"))\n";
        ASSERT_EQUALS("\na(\"foo(\\\"\\\\\\\"\\\")\",foo(\"\\\"\"))\n", OurPreprocessor::expandMacros(filedata));
    }

    void pragma()
    {
        const char filedata[] = "#pragma once\n"
                                "void f()\n"
                                "{\n"
                                "}\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Preprocessor preprocessor;
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS(1, static_cast<unsigned int>(actual.size()));
        ASSERT_EQUALS("\nvoid f()\n{\n}\n", actual[""]);
    }

    void pragma_asm()
    {
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
        Preprocessor preprocessor;
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS(1, static_cast<unsigned int>(actual.size()));
        ASSERT_EQUALS("\n\n\naaa\n\n\n\nbbb\n", actual[""]);
    }

    void endifsemicolon()
    {
        const char filedata[] = "void f()\n"
                                "{\n"
                                "#ifdef A\n"
                                "#endif;\n"
                                "}\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Preprocessor preprocessor;
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS(2, static_cast<unsigned int>(actual.size()));
        ASSERT_EQUALS("void f()\n"
                      "{\n"
                      "\n"
                      "\n"
                      "}\n", actual[""]);
    }

    void handle_error()
    {
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
            Preprocessor preprocessor;
            preprocessor.preprocess(istr, actual, "file.c");

            ASSERT_EQUALS(expected, actual[""]);
            ASSERT_EQUALS("", errout.str());
        }
    }

    void missing_doublequote()
    {
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

            ASSERT_EQUALS("\n\nint a = 1;\n", actual);
            ASSERT_EQUALS("", errout.str());
        }
    }

    void unicodeInCode()
    {
        const std::string filedata(std::string("a") + char(200));
        std::istringstream istr(filedata);
        ASSERT_THROW(Preprocessor::read(istr), std::runtime_error);
    }

    void unicodeInComment()
    {
        const std::string filedata(std::string("//") + char(200));
        std::istringstream istr(filedata.c_str());
        ASSERT_EQUALS("", Preprocessor::read(istr));
    }

    void unicodeInString()
    {
        const std::string filedata(std::string("\"") + char(200) + "\"");
        std::istringstream istr(filedata.c_str());
        ASSERT_EQUALS(filedata, Preprocessor::read(istr));
    }


    void define_part_of_func()
    {
        errout.str("");
        const char filedata[] = "#define A g(\n"
                                "void f() {\n"
                                "  A );\n"
                                "  }\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Preprocessor preprocessor;
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS(1, static_cast<unsigned int>(actual.size()));
        ASSERT_EQUALS("\nvoid f() {\ng( );\n}\n", actual[""]);
        ASSERT_EQUALS("", errout.str());
    }

    void conditionalDefine()
    {
        const char filedata[] = "#ifdef A\n"
                                "#define N 10\n"
                                "#else\n"
                                "#define N 20\n"
                                "#endif\n"
                                "N";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Preprocessor preprocessor;
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS(2, static_cast<unsigned int>(actual.size()));
        ASSERT_EQUALS("\n\n\n\n\n20\n", actual[""]);
        ASSERT_EQUALS("\n\n\n\n\n10\n", actual["A"]);
        ASSERT_EQUALS("", errout.str());
    }


    void multiline_comment()
    {
        errout.str("");
        const char filedata[] = "#define ABC {// \\\n"
                                "}\n"
                                "void f() ABC }\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Preprocessor preprocessor;
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS(1, static_cast<unsigned int>(actual.size()));
        ASSERT_EQUALS("\n\nvoid f() { }\n", actual[""]);
        ASSERT_EQUALS("", errout.str());
    }

    void macro_parameters()
    {
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
        Preprocessor preprocessor(0, this);
        preprocessor.preprocess(istr, actual, "file.c", std::list<std::string>());

        // Compare results..
        ASSERT_EQUALS(1, static_cast<unsigned int>(actual.size()));
        ASSERT_EQUALS("", actual[""]);
        ASSERT_EQUALS("[file.c:6]: (error) Syntax error. Not enough parameters for macro 'BC'.\n", errout.str());
    }

    void newline_in_macro()
    {
        errout.str("");
        const char filedata[] = "#define ABC(str) printf( str )\n"
                                "void f()\n"
                                "{\n"
                                "  ABC(\"\\n\");\n"
                                "}\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Preprocessor preprocessor(0, this);
        preprocessor.preprocess(istr, actual, "file.c", std::list<std::string>());

        // Compare results..
        ASSERT_EQUALS(1, static_cast<unsigned int>(actual.size()));
        ASSERT_EQUALS("\nvoid f()\n{\nprintf(\"\\n\");\n}\n", actual[""]);
        ASSERT_EQUALS("", errout.str());
    }

    void includes()
    {
        {
            std::string src = "#include a.h";
            ASSERT_EQUALS(0, OurPreprocessor::getHeaderFileName(src));
            ASSERT_EQUALS("", src);
        }

        {
            std::string src = "#include \"b.h\"";
            ASSERT_EQUALS(1, OurPreprocessor::getHeaderFileName(src));
            ASSERT_EQUALS("b.h", src);
        }

        {
            std::string src = "#include <c.h>";
            ASSERT_EQUALS(2, OurPreprocessor::getHeaderFileName(src));
            ASSERT_EQUALS("c.h", src);
        }
    }

    void ifdef_ifdefined()
    {
        const char filedata[] = "#ifdef ABC\n"
                                "A\n"
                                "#endif\t\n"
                                "#if defined ABC\n"
                                "A\n"
                                "#endif\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Preprocessor preprocessor;
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS("\n\n\n\n\n\n", actual[""]);
        ASSERT_EQUALS("\nA\n\n\nA\n\n", actual["ABC"]);
        ASSERT_EQUALS(2, static_cast<unsigned int>(actual.size()));
    }

    void define_ifdef()
    {
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
            Preprocessor preprocessor;
            preprocessor.preprocess(istr, actual, "file.c");

            // Compare results..
            ASSERT_EQUALS("\n\n\n\nB\n\n", actual[""]);
            ASSERT_EQUALS(1, actual.size());
        }

        {
            const char filedata[] = "#define A 1\n"
                                    "#ifdef A\n"
                                    "A\n"
                                    "#endif\n";

            // Preprocess => actual result..
            std::istringstream istr(filedata);
            std::map<std::string, std::string> actual;
            Preprocessor preprocessor;
            preprocessor.preprocess(istr, actual, "file.c");

            // Compare results..
            ASSERT_EQUALS("\n\n1\n\n", actual[""]);
            ASSERT_EQUALS(1, actual.size());
        }

        {
            const char filedata[] = "#define A 1\n"
                                    "#if A==1\n"
                                    "A\n"
                                    "#endif\n";

            // Preprocess => actual result..
            std::istringstream istr(filedata);
            std::map<std::string, std::string> actual;
            Preprocessor preprocessor;
            preprocessor.preprocess(istr, actual, "file.c");

            // Compare results..
            TODO_ASSERT_EQUALS("\n\n1\n\n", actual[""]);
            TODO_ASSERT_EQUALS(1, actual.size());
        }

        {
            const char filedata[] = "#define A 1\n"
                                    "#ifdef A>0\n"
                                    "A\n"
                                    "#endif\n";

            // Preprocess => actual result..
            std::istringstream istr(filedata);
            std::map<std::string, std::string> actual;
            Preprocessor preprocessor;
            preprocessor.preprocess(istr, actual, "file.c");

            // Compare results..
            TODO_ASSERT_EQUALS("\n\n1\n\n", actual[""]);
            TODO_ASSERT_EQUALS(1, actual.size());
        }
    }

    void endfile()
    {
        const char filedata[] = "char a[] = \"#endfile\";\n"
                                "char b[] = \"#endfile\";\n"
                                "#include \"notfound.h\"\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Preprocessor preprocessor;
        preprocessor.preprocess(istr, actual, "file.c");

        // Compare results..
        ASSERT_EQUALS("char a[] = \"#endfile\";\nchar b[] = \"#endfile\";\n\n", actual[""]);
        ASSERT_EQUALS(1, actual.size());
    }
};

REGISTER_TEST(TestPreprocessor)
