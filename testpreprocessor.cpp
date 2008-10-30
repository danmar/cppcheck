/*
 * c++check - c/c++ syntax checking
 * Copyright (C) 2007 Daniel Marjamäki
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
#include "preprocessor.h"

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
        TEST_CASE( test1 );
        TEST_CASE( test2 );
        TEST_CASE( test3 );
        TEST_CASE( test4 );
        TEST_CASE( test5 );

        TEST_CASE( comments1 );

        TEST_CASE( if0 );
        TEST_CASE( if1 );

        TEST_CASE( include1 );

        TEST_CASE( if_cond1 );
    }

    bool cmpmaps(const std::map<std::string, std::string> &m1, const std::map<std::string, std::string> &m2)
    {
        // Begin by checking the sizes
        if ( m1.size() != m2.size() )
            return false;

        // Check each item in the maps..
        for ( std::map<std::string,std::string>::const_iterator it1 = m1.begin(); it1 != m1.end(); ++it1 )
        {
            std::string s1 = it1->first;
            std::map<std::string,std::string>::const_iterator it2 = m2.find(s1);
            if ( it2 == m2.end() )
                return false;
            else
            {
                std::string s1 = it1->second;
                std::string s2 = it2->second;
                if ( s1 != s2 )
                    return false;
            }
        }

        // No diffs were found
        return true;
    }


    void test1()
    {
        const char filedata[] = "#ifdef  WIN32 \n"
                                "    abcdef\n"
                                "#else\n"
                                "    qwerty\n"
                                "#endif\n";

        // Expected result..
        std::map<std::string, std::string> expected;
        expected[""]      = "\n\n\nqwerty\n\n";
        expected["WIN32"] = "\nabcdef\n\n\n\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        preprocess( istr, actual );

        // Compare results..
        ASSERT_EQUALS( true, cmpmaps(actual, expected));
    }

    void test2()
    {
        const char filedata[] = "# ifndef WIN32\n"
                                "    \" # ifdef WIN32\" // a comment\n"
                                "   #   else  \n"
                                "    qwerty\n"
                                "  # endif  \n";

        // Expected result..
        std::map<std::string, std::string> expected;
        expected["WIN32"] = "\n\n\nqwerty\n\n";
        expected[""]      = "\n\" # ifdef WIN32\"\n\n\n\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        preprocess( istr, actual );

        // Compare results..
        ASSERT_EQUALS( true, cmpmaps(actual, expected));
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

        // Expected result..
        std::map<std::string, std::string> expected;
        expected[""]        = "\n\n\n\n\n\n\n";
        expected["ABC"]     = "\na\n\n\n\nc\n\n";
        expected["ABC;DEF"] = "\na\n\nb\n\nc\n\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        preprocess( istr, actual );

        // Compare results..
        ASSERT_EQUALS( true, cmpmaps(actual, expected));
    }

    void test4()
    {
        const char filedata[] = "#ifdef ABC\n"
                                "A\n"
                                "#endif\n"
                                "#ifdef ABC\n"
                                "A\n"
                                "#endif\n";

        // Expected result..
        std::map<std::string, std::string> expected;
        expected[""]        = "\n\n\n\n\n\n";
        expected["ABC"]     = "\nA\n\n\nA\n\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        preprocess( istr, actual );

        // Compare results..
        ASSERT_EQUALS( true, cmpmaps(actual, expected));
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

        // Expected result..
        std::map<std::string, std::string> expected;
        expected[""]    = "\n\n\nB\n\n\n\n\n";
        expected["ABC"] = "\nA\n\n\n\n\n\n\n";
        expected["DEF"] = "\n\n\nB\n\nC\n\n\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        preprocess( istr, actual );

        // Compare results..
        ASSERT_EQUALS( true, cmpmaps(actual, expected));
    }



    void comments1()
    {
        const char filedata[] = "/*\n"
                                "#ifdef WIN32\n"
                                "#endif\n"
                                "*/\n";

        // Expected result..
        std::map<std::string, std::string> expected;
        expected[""] = "\n\n\n\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        preprocess( istr, actual );

        // Compare results..
        ASSERT_EQUALS( true, cmpmaps(actual, expected));
    }



    void if0()
    {
        const char filedata[] = " # if /* comment */  0 // comment\n"
                                "#ifdef WIN32\n"
                                "#endif\n"
                                "#endif\n";

        // Expected result..
        std::map<std::string, std::string> expected;
        expected[""] = "\n\n\n\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        preprocess( istr, actual );

        // Compare results..
        ASSERT_EQUALS( true, cmpmaps(actual, expected));
    }

    void if1()
    {
        const char filedata[] = " # if /* comment */  1 // comment\n"
                                "ABC\n"
                                " # endif \n";

        // Expected result..
        std::map<std::string, std::string> expected;
        expected[""] = "\nABC\n\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        preprocess( istr, actual );

        // Compare results..
        ASSERT_EQUALS( true, cmpmaps(actual, expected));
    }



    void include1()
    {
        const char filedata[] = " # include \"abcd.h\" // abcd\n";

        // Expected result..
        std::map<std::string, std::string> expected;
        expected[""] = "#include \"abcd.h\"\n";

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        preprocess( istr, actual );

        // Compare results..
        ASSERT_EQUALS( true, cmpmaps(actual, expected));
    }




    void if_cond1()
    {
        /* TODO: Make this work
        const char filedata[] = "#if LIBVER>100\n"
                                "    A\n"
                                "#else\n"
                                "    B\n"
                                "#endif\n";

        std::map<std::string, std::string> expected;
        expected[""] = filedata;

        // Preprocess => actual result..
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        preprocess( istr, actual );

        // Compare results..
        ASSERT_EQUALS( true, cmpmaps(actual, expected));
        */
    }

};

REGISTER_TEST( TestPreprocessor )
