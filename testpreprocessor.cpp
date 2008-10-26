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
    }

    void check(const char filedata[], const std::map<std::string,std::string> &expected)
    {
        std::istringstream istr(filedata);
        std::map<std::string, std::string> actual;
        preprocess( istr, actual );

        ASSERT_EQUALS( expected.size(), actual.size() );
        for ( std::map<std::string,std::string>::const_iterator it = actual.begin(); it != actual.end(); ++it )
        {
            std::map<std::string,std::string>::const_iterator it2 = expected.find(it->first);
            if ( it2 == expected.end() )
                assertFail(__FILE__, __LINE__);
            else
            {
                std::string s1 = it->second;
                std::string s2 = it2->second;
                ASSERT_EQUALS( it->second, it2->second );
            }
        }
    }

    void test1()
    {
        const char filedata[] = "#ifdef WIN32\n"
                                "    abcdef\n"
                                "#else\n"
                                "    qwerty\n"
                                "#endif\n";

        std::map<std::string, std::string> expected;
        expected[""]      = "\n\n\n    qwerty\n\n";
        expected["WIN32"] = "\n    abcdef\n\n\n\n";

        check( filedata, expected );
    }

    void test2()
    {
        const char filedata[] = "#ifndef WIN32\n"
                                "    abcdef\n"
                                "#else\n"
                                "    qwerty\n"
                                "#endif\n";

        std::map<std::string, std::string> expected;
        expected[""]      = "\n    abcdef\n\n\n\n";
        expected["WIN32"] = "\n\n\n    qwerty\n\n";

        check( filedata, expected );
    }

    void comments1()
    {
        const char filedata[] = "/*\n"
                                "#ifdef WIN32\n"
                                "#endif\n"
                                "*/\n";

        std::map<std::string, std::string> expected;
        expected[""] = "\n\n\n\n";
        check( filedata, expected );
    }

};

REGISTER_TEST( TestPreprocessor )
