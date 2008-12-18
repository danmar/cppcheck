/*
 * c++check - c/c++ syntax checking
 * Copyright (C) 2008 Daniel Marjamäki and Reijo Tomperi
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

#include <string>
#include "testsuite.h"
#include "filelister.h"

class TestFileLister : public TestFixture
{
public:
    TestFileLister() : TestFixture("TestFileLister")
    { }

private:

    void run()
    {
        TEST_CASE( simplify_path );
    }

    void simplify_path()
    {
        ASSERT_EQUALS( std::string( "index.h" ), FileLister::simplifyPath( "index.h" ) );
        ASSERT_EQUALS( std::string( "/index.h" ), FileLister::simplifyPath( "/index.h" ) );
        ASSERT_EQUALS( std::string( "/path/" ), FileLister::simplifyPath( "/path/" ) );
        ASSERT_EQUALS( std::string( "/" ), FileLister::simplifyPath( "/" ) );
        ASSERT_EQUALS( std::string( "./index.h" ), FileLister::simplifyPath( "./index.h" ) );
        ASSERT_EQUALS( std::string( "../index.h" ), FileLister::simplifyPath( "../index.h" ) );
        ASSERT_EQUALS( std::string( "/index.h" ), FileLister::simplifyPath( "/path/../index.h" ) );
        ASSERT_EQUALS( std::string( "/index.h" ), FileLister::simplifyPath( "/path/../other/../index.h" ) );
        ASSERT_EQUALS( std::string( "/index.h" ), FileLister::simplifyPath( "/path/../other///././../index.h" ) );
        ASSERT_EQUALS( std::string( "../path/index.h" ), FileLister::simplifyPath( "../path/other/../index.h" ) );
    }


};

REGISTER_TEST( TestFileLister )
