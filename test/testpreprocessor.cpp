/*
 * cppcheck - c/c++ syntax checking
 * Copyright (C) 2007-2009 Daniel Marjamäki, Reijo Tomperi, Nicolas Le Cam
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/
 */


// The preprocessor that c++check uses is a bit special. Instead of generating
// the code for a known configuration, it generates the code for each configuration.


#include "testsuite.h"
#define UNIT_TESTING
#include "../src/preprocessor.h"

#include <map>
#include <string>

class TestPreprocessor : public TestFixture
{
public:
    TestPreprocessor() : TestFixture("TestPreprocessor")
    { }

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

        TEST_CASE(newlines);

        TEST_CASE(comments1);

        TEST_CASE(if0);
        TEST_CASE(if1);

        TEST_CASE(elif);

        TEST_CASE(if_cond1);

        TEST_CASE(multiline1);
        TEST_CASE(multiline2);
        TEST_CASE(multiline3);

        TEST_CASE(if_defined);      // "#if defined(AAA)" => "#ifdef AAA"

        // Macros..
        TEST_CASE(macro_simple1);
        TEST_CASE(macro_simple2);
        TEST_CASE(macro_simple3);
        TEST_CASE(macro_simple4);
        TEST_CASE(macro_simple5);
        TEST_CASE(macro_simple6);
        TEST_CASE(macro_mismatch);
        TEST_CASE(string1);
        TEST_CASE(string2);
        TEST_CASE(preprocessor_undef);
        TEST_CASE(preprocessor_doublesharp);
        TEST_CASE(preprocessor_include_in_str);
        // TODO TEST_CASE(fmt);
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
        preprocessor.preprocess(istr, actual);

        // Compare results..
        ASSERT_EQUALS(expected[""], actual[""]);
        ASSERT_EQUALS(expected["__cplusplus"], actual["__cplusplus"]);
        ASSERT_EQUALS(2, actual.size());
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
        preprocessor.preprocess(istr, actual);

        // Compare results..
        ASSERT_EQUALS("\n\n\nqwerty\n\n", actual[""]);
        ASSERT_EQUALS("\nabcdef\n\n\n\n", actual["WIN32"]);
        ASSERT_EQUALS(2, actual.size());
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
        preprocessor.preprocess(istr, actual);

        // Compare results..
        ASSERT_EQUALS("\n\" # ifdef WIN32\"\n\n\n\n", actual[""]);
        ASSERT_EQUALS("\n\n\nqwerty\n\n", actual["WIN32"]);
        ASSERT_EQUALS(2, actual.size());
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
        preprocessor.preprocess(istr, actual);

        // Compare results..
        ASSERT_EQUALS("\n\n\n\n\n\n\n", actual[""]);
        ASSERT_EQUALS("\na\n\n\n\nc\n\n", actual["ABC"]);
        ASSERT_EQUALS("\na\n\nb\n\nc\n\n", actual["ABC;DEF"]);
        ASSERT_EQUALS(3, actual.size());
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
        preprocessor.preprocess(istr, actual);

        // Compare results..
        ASSERT_EQUALS("\n\n\n\n\n\n", actual[""]);
        ASSERT_EQUALS("\nA\n\n\nA\n\n", actual["ABC"]);
        ASSERT_EQUALS(2, actual.size());
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
        preprocessor.preprocess(istr, actual);

        // Compare results..
        ASSERT_EQUALS("\n\n\nB\n\n\n\n\n", actual[""]);
        ASSERT_EQUALS("\nA\n\n\n\n\n\n\n", actual["ABC"]);
        ASSERT_EQUALS("\n\n\nB\n\nC\n\n\n", actual["DEF"]);
        ASSERT_EQUALS(3, actual.size());
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
        const char filedata[] = "/*\n"
                                "#ifdef WIN32\n"
                                "#endif\n"
                                "*/\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        Preprocessor preprocessor;
        preprocessor.preprocess(istr, actual);

        // Compare results..
        ASSERT_EQUALS("\n\n\n\n", actual[""]);
        ASSERT_EQUALS(1, actual.size());
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
        preprocessor.preprocess(istr, actual);

        // Compare results..
        ASSERT_EQUALS("\n\n\n\n", actual[""]);
        ASSERT_EQUALS(1, actual.size());
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
        preprocessor.preprocess(istr, actual);

        // Compare results..
        ASSERT_EQUALS("\nABC\n\n", actual[""]);
        ASSERT_EQUALS(1, actual.size());
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
        preprocessor.preprocess(istr, actual);

        // Compare results..
        ASSERT_EQUALS("\n\n\n\n\n", actual[""]);
        ASSERT_EQUALS("\nABC\n\n\n\n", actual["DEF1"]);
        ASSERT_EQUALS("\n\n\nDEF\n\n", actual["DEF2"]);
        ASSERT_EQUALS(3, actual.size());
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
        preprocessor.preprocess(istr, actual);

        // Compare results..
        ASSERT_EQUALS("\n\n\nB\n\n", actual[""]);
        ASSERT_EQUALS("\nA\n\n\n\n", actual["LIBVER>100"]);
        ASSERT_EQUALS(2, actual.size());
    }


    void multiline1()
    {
        const char filedata[] = "#define str \"abc\"     \\\n"
                                "            \"def\"       \n"
                                "abcdef = str;\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        Preprocessor preprocessor;
        ASSERT_EQUALS("#define str \"abc\"  \"def\" \n\nabcdef = str;\n", preprocessor.read(istr));
    }

    void multiline2()
    {
        const char filedata[] = "#define sqr(aa) aa * \\\n"
                                "                aa\n"
                                "sqr(5);\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        Preprocessor preprocessor;
        ASSERT_EQUALS("#define sqr(aa) aa *  aa\n\nsqr(5);\n", preprocessor.read(istr));
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



    void if_defined()
    {
        const char filedata[] = "#if defined(AAA)\n"
                                "#endif\n";


        // Expected result..
        std::string expected("#ifdef AAA\n#endif\n");

        // Compare result..
        ASSERT_EQUALS(expected, Preprocessor::replaceIfDefined(filedata));
    }


    void macro_simple1()
    {
        const char filedata[] = "#define AAA(aa) f(aa)\n"
                                "AAA(5);\n";
        ASSERT_EQUALS("\nf(5);\n", Preprocessor::expandMacros(filedata));
    }

    void macro_simple2()
    {
        const char filedata[] = "#define min(x,y) x<y?x:y\n"
                                "min(a(),b());\n";
        ASSERT_EQUALS("\na()<b()?a():b();\n", Preprocessor::expandMacros(filedata));
    }

    void macro_simple3()
    {
        const char filedata[] = "#define A 4\n"
                                "A AA\n";
        ASSERT_EQUALS("\n4 AA\n", Preprocessor::expandMacros(filedata));
    }

    void macro_simple4()
    {
        const char filedata[] = "#define TEMP_1 if( temp > 0 ) return 1;\n"
                                "TEMP_1\n";
        ASSERT_EQUALS("\nif( temp > 0 ) return 1;\n", Preprocessor::expandMacros(filedata));
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
        ASSERT_EQUALS("\n\nvoid foo()\n{\n    int temp = 0;\n    if( temp > 0 ) return 1;\n}\n", Preprocessor::expandMacros(filedata));
    }

    void macro_simple6()
    {
        const char filedata[] = "#define ABC (a+b+c)\n"
                                "ABC";
        ASSERT_EQUALS("\n(a+b+c)", Preprocessor::expandMacros(filedata));
    }

    void macro_mismatch()
    {
        const char filedata[] = "#define AAA(aa,bb) f(aa)\n"
                                "AAA(5);\n";
        ASSERT_EQUALS("\nAAA(5);\n", Preprocessor::expandMacros(filedata));
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
        preprocessor.preprocess(istr, actual);

        // Compare results..
        ASSERT_EQUALS(1, actual.size());
        ASSERT_EQUALS("int main(){ const char *a = \"#define A\n\";}\n", actual[""]);
    }

    void string2()
    {
        const char filedata[] = "#define AAA 123\n"
                                "str = \"AAA\"";

        // Compare results..
        ASSERT_EQUALS("\nstr = \"AAA\"", Preprocessor::expandMacros(filedata));
    }


    void preprocessor_undef()
    {
        const char filedata[] = "#define AAA int a;\n"
                                "#undef AAA\n"
                                "#define AAA char b=0;\n"
                                "AAA\n";

        // Compare results..
        ASSERT_EQUALS("\n\n\nchar b=0;\n", Preprocessor::expandMacros(filedata));
    }

    void preprocessor_doublesharp()
    {
        const char filedata[] = "#define TEST(var,val) var = val\n"
                                "TEST(foo,20);\n";

        // Compare results..
        ASSERT_EQUALS("\nfoo=20;\n", Preprocessor::expandMacros(filedata));

        const char filedata2[] = "#define TEST(var,val) var##_##val = val\n"
                                 "TEST(foo,20);\n";

        // Compare results..
        ASSERT_EQUALS("\nfoo_20=20;\n", Preprocessor::expandMacros(filedata2));
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
        preprocessor.preprocess(istr, actual);

        // Compare results..
        ASSERT_EQUALS(1, actual.size());
        ASSERT_EQUALS("int main()\n{\nconst char *a = \"#include <string>\n\";\nreturn 0;\n}\n", actual[""]);
    }


    void fmt()
    {
        const char filedata[] = "#define DBG(fmt...) printf(fmt);\n"
                                "DBG(\"[0x%lx-0x%lx)\", pstart, pend);";

        // Preprocess..
        std::string actual = Preprocessor::expandMacros(filedata);
    }

};

REGISTER_TEST(TestPreprocessor)
