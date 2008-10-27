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

        TEST_CASE( comments1 );

        TEST_CASE( if0 );
    }

    bool cmpmaps(const std::map<std::string, std::string> &m1, const std::map<std::string, std::string> &m2)
    {
        // Begin by checking the sizes
        if ( m1.size() != m2.size() )
            return false;
            
        // Check each item in the maps..
        for ( std::map<std::string,std::string>::const_iterator it1 = m1.begin(); it1 != m1.end(); ++it1 )
        {
            std::map<std::string,std::string>::const_iterator it2 = m2.find(it1->first);
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
                                "    \"#ifdef WIN32\" // a comment\n"
                                "   #   else  \n"
                                "    qwerty\n"
                                "  # endif  \n";

        // Expected result..
        std::map<std::string, std::string> expected;
        expected[""]      = "\n\"............\"\n\n\n\n";
        expected["WIN32"] = "\n\n\nqwerty\n\n";

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


};

REGISTER_TEST( TestPreprocessor )
